#undef PUBSUB__FILE__
#define PUBSUB__FILE__ "main.c"
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
    loop = EV_DEFAULT;
    ev_timer timer_w;
    ev_io accept_w;
    ev_signal signal_w;

    if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        fprintf(stderr, "Event-based MQTT broker.\n");
        exit(EXIT_FAILURE);
    }

    listen_fd = net_init(argv[1]);
    if (listen_fd == -1)
        exit(1);

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
