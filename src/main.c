#undef PUBSUB__FILE__
#define PUBSUB__FILE__ "main.c"
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <ev.h>

#include "main.h"
#include "data.h"
#include "net.h"
#include "log.h"
#include "sig.h"
#include "misc.h"

struct ev_loop *loop;
struct clients_head clients = SLIST_HEAD_INITIALIZER(clients);
unsigned int num_clients = 0;
int listen_fd;

int main(int argc, char *argv[]) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret;
    loop = EV_DEFAULT;
    ev_timer timer_w;
    ev_io accept_w;
    ev_signal signal_w;

    if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        fprintf(stderr, "Event-based MQTT broker.\n");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    ret = getaddrinfo(NULL, argv[1] /* port */, &hints, &result);
    if (ret != 0) {
        logmsg(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listen_fd == -1)
            continue;
        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(listen_fd);
    }
    freeaddrinfo(result);

    if (rp == NULL) {
        logmsg(LOG_ERR, "could not bind socket\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 5 /* pending conns */) == -1) {
        logmsg(LOG_ERR, "could not listen on port %d\n", atoi(argv[1]));
        exit(EXIT_FAILURE);
    }

    fcntl(listen_fd, F_SETFL, fcntl(listen_fd, F_GETFL, 0) | O_NONBLOCK);
    logmsg(LOG_INFO, "listening on port %d\n", atoi(argv[1]));

    LIST_INIT(&clients);

    /* start incoming request watcher */
    ev_io_init(&accept_w, accept_cb, listen_fd, EV_READ);
    ev_io_start(EV_A_ &accept_w);

    /* start statistics and cleanup watcher */
    ev_timer_init(&timer_w, timeout_cb, 0., 60.);
    ev_timer_start(EV_A_ &timer_w);

    /* start signal watcher */
    ev_signal_init(&signal_w, sig_cb, SIGINT);
    ev_signal_start(EV_A_ &signal_w);

    /* unregister from loop to prevent keeping it spinning madly */
    ev_unref(EV_A);

    ev_run(EV_A_ 0);
    logmsg(LOG_INFO, "exiting…\n");

    return 0;
}
