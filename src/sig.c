
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
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
        logmsg(LOG_ERR, "could not close socket: %s\n", gai_strerror(ret));

    /* disconnect all clients */
    LIST_FOREACH(client, &clients, entries) {
        LIST_REMOVE(client, entries);
        close(client->peer_fd);
        free(client->identifier); /* XXX: this may not have been malloced() yet */
        free(client->will_topic); /* XXX: this may not have been malloced() yet */
        free(client->will_msg);   /* XXX: this may not have been malloced() yet */
        free(client->inbuf);
        free(client);
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
