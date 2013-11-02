
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ev.h>

#include "log.h"
#include "main.h"
#include "data.h"

void sig_cb(EV_P_ ev_signal *signal_w, int revents) {
    int ret;
    struct Client *client;

    logmsg(LOG_INFO, "received SIGINT, shutting down\n");

    if ((ret = close(listen_fd)) != 0)
        logmsg(LOG_ERR, "could not close socket: %s\n", strerror(errno));

    /* disconnect all clients */
    LIST_FOREACH(client, &clients, entries) {
        close(client->fd);
        free_client(client);
    }

    /*ev_io_stop(loop, *data.accept_w);
    free(accept_w);
    ev_timer_stop(loop, &timer_w);
    free(&timer_w);
    ev_signal_stop(loop, signal_w);
    free(&signal_w);*/
    ev_break(EV_A_ EVBREAK_ALL);
    /*free(&loop);*/
}
