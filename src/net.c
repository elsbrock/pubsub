
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ev.h>
#include <assert.h>

#include "queue.h"

#include "net.h"
#include "log.h"
#include "data.h"
#include "main.h"
#include "mqtt.h"
#include "util.h"

static int read_packet(struct Client *client);
static void client_read_cb(EV_P_ struct ev_io *peer_w, int revents);
static void client_write_cb(EV_P_ struct ev_io *peer_w, int revents);

int net_init(const char *port) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret;
    int fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    ret = getaddrinfo(NULL, port, &hints, &result);
    if (ret != 0) {
        logmsg(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1)
            continue;
        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(fd);
    }
    freeaddrinfo(result);

    if (rp == NULL) {
        logmsg(LOG_ERR, "could not bind socket\n");
        return -1;
    }

    if (listen(fd, 5 /* pending conns */) == -1) {
        logmsg(LOG_ERR, "could not listen on port %d\n", atoi(port));
        return -1;
    }

    fcntl(fd, F_SETFL, fcntl(listen_fd, F_GETFL, 0) | O_NONBLOCK);
    logmsg(LOG_INFO, "listening on port %d\n", atoi(port));
    return fd;
}

/* Accepts a single incoming connection on the listen_fd. If the connection
 * could be accepted, a new Client is allocated and appended to the client
 * list. In addition, a new ev_io watcher is registered and started for READ
 * events.
 */
void accept_cb(EV_P_ struct ev_io *watcher, int revents) {
    if (EV_ERROR & revents) {
        logmsg(LOG_ERR, "libev: invalid event\n");
        return;
    }

    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int peer_sd = accept(watcher->fd, (struct sockaddr *)&peer_addr, &peer_len);
    if (peer_sd < 0) {
        if (errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK)
            logmsg(LOG_ERR, "could not accept connection: %s\n", strerror(errno));
        return;
    }

    char host[NI_MAXHOST], service[NI_MAXSERV];
    int ret = getnameinfo((struct sockaddr *) &peer_addr, peer_len, host,
                NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);

    if (ret == 0)
        logmsg(LOG_INFO, "new connection by %s:%d\n", host, atoi(service));
    else
        logmsg(LOG_ERR, "getnameinfo: %s\n", gai_strerror(ret));

    struct Client *client = smalloc(sizeof(struct Client));

    client->fd = peer_sd;
    client->state = S_CONNECTING;
    client->inbuf = smalloc(sizeof(char) * BUF_LEN);
    client->inbuf_bytes = 0;

    client->identifier = NULL;
    client->will_topic = NULL;
    client->will_msg = NULL;

    client->outgoing_num = 0;
    LIST_INIT(&(client->outgoing_msgs));

    client->read_w  = smalloc(sizeof(struct ev_io));
    client->write_w = smalloc(sizeof(struct ev_io));

    ev_io_init(client->read_w, client_read_cb, peer_sd, EV_READ);
    ev_io_start(EV_A_ client->read_w);

    ev_io_init(client->write_w, client_write_cb, peer_sd, EV_WRITE);

    LIST_INSERT_HEAD(&clients, client, entries);
    num_clients++;
}

static void client_read_cb(EV_P_ struct ev_io *read_w, int revents) {
    if ((revents & EV_READ) == 0)
        return;

    ssize_t bytes_read;
    int ret = 0;

    /* look up client context */
    /* XXX: use hashtable? */
    Client *client = NULL;
    LIST_FOREACH(client, &clients, entries) {
        if (client->fd == read_w->fd)
            break;
    }

    assert(client != NULL);
    assert(client->read_w->fd == client->fd);

    bytes_read = read(client->fd, client->inbuf+client->inbuf_bytes,
            BUF_LEN-client->inbuf_bytes);
    if (bytes_read == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            logmsg(LOG_DEBUG, "read() failed: %s\n", strerror(errno));
        return;
    }
    if (bytes_read == 0) {
        logmsg(LOG_INFO, "client disconnected\n");
        num_clients--;

        if (close(client->fd) == -1)
            logmsg(LOG_ERR, "could not close socket: %s\n", strerror(errno));

        free_client(client);
        return;
    }

    logmsg(LOG_DEBUG, "read %d bytes from client\n", bytes_read);

    client->inbuf_bytes += bytes_read;
    if (client->inbuf_bytes >= 2)
        ret = read_packet(client);

    /* XXX: free and malloc() again if msg was > 4096 */
    if (ret == 1)
        client->inbuf_bytes = 0;
    else if (ret == -1)
        /* In case of a complete message of unknown type, discard it. */
        client->inbuf_bytes = 0;
}

static void client_write_cb(EV_P_ struct ev_io *write_w, int revents) {
    if ((revents & EV_WRITE) == 0)
        return;

    Client *client;
    ssize_t ret;

    /* look up client context */
    /* XXX: use hashtable? */
    LIST_FOREACH(client, &clients, entries) {
        if (client->fd == client->fd)
            break;
    }

    assert(client != NULL);
    assert(client->write_w->fd == client->fd);

    /* Try to send out a single message. */
    Envelope *envelope;
    LIST_FOREACH(envelope, &client->outgoing_msgs, entries) {
        assert(envelope->bytes_total != envelope->bytes_sent);

        logmsg(LOG_DEBUG, "going to send %d bytes of %d total bytes\n",
                envelope->bytes_total-envelope->bytes_sent, envelope->bytes_total);

        ret = write(client->fd, envelope->msg+envelope->bytes_sent,
                envelope->bytes_total-envelope->bytes_sent);

        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                break;
            /* else {
                disconnect_client();
                return;
            */
            return;
        }

        envelope->bytes_sent += ret;

        /* If the entire message could be sent, remove it from the queue. */
        if (envelope->bytes_sent == envelope->bytes_total) {
            /* XXX: if the envelope is shared, don't free it */
            logmsg(LOG_DEBUG, "msg sent!\n");
            LIST_REMOVE(envelope, entries);
            free_envelope(envelope);
            client->outgoing_num--;
        }

        break; /* maybe send more? */
    }

    /* If there are no pending outgoing messages in the queue left, disable the
     * write watcher to avoid busy looping. */
    if (client->outgoing_num == 0)
        ev_io_stop(EV_A_ write_w);
}

/* Reads a potentially incomplete message of least 2 bytes size. Parsing the
 * first two bytes allows us to get the message type, whereas the second byte
 * is needed to decide whether the packet is already complete or not. If it is
 * complete, the packet is passed over to one of the message handlers, where it
 * can be processed.
 *
 * Returns 1 if the processing succeeded, 0 if the message is not yet complete
 * and -1 if this message is either unknown or if there is no handler for this
 * type of message.
 */
static int read_packet(struct Client *client) {
    assert(client->inbuf_bytes >= 2);

    size_t msg_length = 0;
    bool msg_complete = false;

    /* Variable length encoding: seven bits encode the remaining length,
     * the eigth bit is the continuation indicator. The maximum number of
     * bytes is limited to four.                       (Source: mqtt ref)
     */
    unsigned int msg_index = 1;
    unsigned int multiplier = 1;
    unsigned int thisbyte;
    do {
        thisbyte = client->inbuf[msg_index++];
        msg_length = (thisbyte & 0x7F) * multiplier;
        multiplier *= 128;
    } while (msg_index < client->inbuf_bytes && (thisbyte & 0x7F) != 0);

    if (client->inbuf_bytes-2 /* header */ == msg_length)
        msg_complete = true;

    if (msg_complete)
        logmsg(LOG_DEBUG, "message is complete (%d bytes)\n", client->inbuf_bytes);
    else {
        /* XXX: the buffer is pretty small (4096 bytes), so messages with
         * payload > 4096 bytes will never be handled at the moment. */
        logmsg(LOG_DEBUG, "expected %d bytes, but got %d so far\n", msg_length,
                client->inbuf_bytes);
        assert(client->inbuf_bytes < msg_length);
        return 0; /* try again later */
    }

    int ret = 0;
    msg_t type = (client->inbuf[0]) & 0xF0;
    if (client->state == S_CONNECTING && type != T_CONNECT) {
        /* XXX: disconnect client */
        logmsg(LOG_ERR, "invalid client state: expected CONNECT message but got 0x%x\n", type);
        return 0;
    }

    switch(type) {
        case T_CONNECT:
            logmsg(LOG_DEBUG, "CONNECT from client\n");
            ret = handle_connect(client, msg_length);
            break;
        case T_PINGREQ:
            logmsg(LOG_DEBUG, "PINGREQ from client\n");
            ret = handle_pingreq(client, msg_length);
            break;
        default:
            logmsg(LOG_DEBUG, "unhandled message type: 0x%x\n", type);
            ret = -1;
            break;
    }

    /*if (ret)
        reset_keepalive(client);*/

    logmsg(LOG_DEBUG, "read_packet=%d\n", ret);
    return ret;
}
