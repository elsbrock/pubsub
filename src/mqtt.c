
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>

#include "main.h"
#include "log.h"
#include "data.h"
#include "mqtt.h"
#include "util.h"

static int create_msg(Message *msg, msg_t type, uint8_t qos, bool retain, size_t len);
static int enqueue_msg(Client *client, Message *msg);

/* Handles a CONNECT message. Assumes that the message is complete. */
int handle_connect(Client *client, size_t msg_len) {
    assert(client->state == S_CONNECTING);
    assert((client->inbuf[0] & 0xF0) >> 4 == 1); 
    assert((client->inbuf[0] & 0xF) == 0); /* DUP, QoS & Retain */

    int len, walk = 2;

    /* protocol identifier */
    len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;

    /* XXX: change these to IFs and return appropriate MSG to client */
    assert(len == 6);
    assert(strncmp(client->inbuf+walk, PROTOCOL_NAME, len) == 0);
    walk += len;
    assert(client->inbuf[walk] == PROTOCOL_VERSION); /* protocol version */
    walk++;

    /* connect flags */
    uint8_t flags = client->inbuf[walk];

    bool has_username = (flags & 0x80) == 1 << 7;
    bool has_password = (flags & 0x40) == 1 << 6;
    bool will_retain  = (flags & 0x20) == 1 << 5;
    uint8_t will_qos  = (flags & 0x18) >> 3; /* 2 bytes */
    bool will_flag    = (flags & 0x04) == 1 << 2;
    bool clean_session = (flags & 0x02) == 1 << 1;
    /* LSB is not used */
    walk++;

    logmsg(LOG_DEBUG, "flags: %x\n", flags);
    logmsg(LOG_DEBUG, "username: %d, password: %d, will_retain: %d, will_qos: %d, will_flag: %d, clean_session: %d\n", has_username, has_password, will_retain, will_qos, will_flag, clean_session);

    /* keep alive (in seconds) */
    client->keepalive = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;
    /* XXX: register keep-alive timer */

    logmsg(LOG_DEBUG, "keepalive is %d seconds\n", client->keepalive);

    /* payload: client identifier */
    len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;

    /* XXX: check if character len < 23 and bail out */
    /* XXX: check uniqueness of identifier */
    client->identifier = smalloc(sizeof(char) * len+1);
    logmsg(LOG_DEBUG, "client identitifer len: %d\n", len);
    memcpy(client->identifier, client->inbuf+walk, len);
    client->identifier[len] = '\0';
    logmsg(LOG_DEBUG, "client identifier: %s\n", client->identifier);
    walk += len;

    if (will_flag) {
        /* XXX: check length of topic and message */

        /* payload: will topic  */
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        client->will_topic = smalloc(sizeof(char) * len+1);
        memcpy(client->will_topic, client->inbuf+walk, len);
        client->will_topic[len] = '\0';
        walk += len;

        /* payload: will message */
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        client->will_msg = smalloc(sizeof(char) * len+1);
        memcpy(client->will_msg, client->inbuf+walk, len);
        client->will_msg[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "will-topic: '%s', will_msg: '%s'\n", client->will_topic, client->will_msg);
    }

    char *username = NULL;
    /* For compatibility reasons, the username flag may be set, although no
     * username is present. Therefore, check if the username really is there.
     */
    if (has_username && client->inbuf_bytes > 2+/* fixed header */walk+2) {
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        username = smalloc(sizeof(char) * len + 1);
        memcpy(username, client->inbuf+walk, len);
        username[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "username: %s\n", username);
    }

    char *password = NULL;
    /* For compatibility reasons, the password flag may be set, although no
     * password is present. Therefore, check if the password really is there.
     */
    if (has_password && client->inbuf_bytes > 2+/* fixed header */walk+2) {
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        password = smalloc(sizeof(char) * len + 1);
        memcpy(password, client->inbuf+walk, len);
        password[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "password: %s\n", password);
    }

    /* XXX: check credentials */
    if (username != NULL)
        free(username);
    if (password != NULL)
        free(password);

    Message *msg = smalloc(sizeof(Message));
    create_msg(msg, T_CONNACK, 0, false, 2 /* variable header */);

    /* XXX: not sure if this is needed, the first byte is reserved (not used) */
    memset(msg->payload, 0, 1);
    msg->payload[1] = R_ACK;

    enqueue_msg(client, msg);
    client->state = S_CONNECTED;

    return 1;
}

int handle_pingreq(Client *client, size_t msg_len) {
    Message *msg = smalloc(sizeof(Message));
    create_msg(msg, T_PINGRESP, 0, false, 0);
    enqueue_msg(client, msg);

    /* XXX: reset keep-alive */
    return 1;
}

/* Initializes an empty MQTT message. */
static int create_msg(Message *msg, msg_t type, uint8_t qos, bool retain, size_t payload_len) {
    /* XXX: check payload_len */

    msg->type = type;
    msg->flags = smalloc(3); /* XXX */
    msg->flags->duplicate = false;
    msg->flags->qos = qos;
    msg->flags->retain = retain;
    msg->remaining_num = 0;

    memset(msg->remaining_bytes, 0, 4);

    uint8_t byte;
    msg->payload_len = payload_len;
    do {
        /* cut MSB */
        byte = payload_len % 128;
        payload_len /= 128;
        if (payload_len > 0) {
            /* set LSB to 1 if more bytes to come */
            byte = byte | 0x80;
        }
        msg->remaining_bytes[msg->remaining_num] = byte;
        msg->remaining_num++;
        if (msg->remaining_num == 4)
            break;
    } while (payload_len > 0);

    msg->payload = smalloc(msg->payload_len);

    return 1;
}

static int enqueue_msg(Client *client, Message *msg) {
    Envelope *envelope = smalloc(sizeof(Envelope));

    envelope->enqueued_at = 0;
    envelope->bytes_sent = 0;
    envelope->bytes_total = msg->payload_len + msg->remaining_num + 1;
    envelope->msg = smalloc(envelope->bytes_total);

    envelope->msg[0] = (msg->type | msg->flags->duplicate << 3
        | msg->flags->qos << 1 | msg->flags->retain);

    /* XXX: well, this is rather stupid. */
    memcpy(envelope->msg+1, msg->remaining_bytes, msg->remaining_num);
    memcpy(envelope->msg+1+msg->remaining_num, msg->payload, msg->payload_len);

    free(msg->payload);
    free(msg->flags);
    free(msg);

    LIST_INSERT_HEAD(&(client->outgoing_msgs), envelope, entries);
    client->outgoing_num++;

    /* XXX: if the message is small, try to avoid starting the watcher by
     * writing directly. */

    if (client->outgoing_num == 1)
        ev_io_start(loop, client->write_w);

    return 1;
}

void free_client(Client *client) {
    LIST_REMOVE(client, entries);
    if (client->identifier != NULL)
        free(client->identifier);
    if (client->will_topic != NULL)
        free(client->will_topic);
    if (client->will_msg != NULL)
        free(client->will_msg);
    free(client->inbuf);

    Envelope *envelope;
    LIST_FOREACH(envelope, &client->outgoing_msgs, entries) {
        /* XXX: if envelope is shared, don't free it */
        free_envelope(envelope);
    }

    ev_io_stop(EV_A_ client->read_w);
    ev_io_stop(EV_A_ client->write_w);
    free(client->read_w);
    free(client->write_w);
    free(client);
}

void free_envelope(Envelope *envelope) {
    free(envelope->msg);
    free(envelope);
}
