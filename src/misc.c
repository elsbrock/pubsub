
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <ev.h>

#include "log.h"
#include "main.h"

void timeout_cb (EV_P_ ev_timer *timer_w, int revents) {
    logmsg(LOG_INFO, "connected clients: %d\n", num_clients);
    ev_timer_again(EV_A_ timer_w);
}
