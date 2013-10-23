
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <ev.h>

#include "log.h"
#include "main.h"
#include "util.h"

void timeout_cb (EV_P_ ev_timer *timer_w, int revents) {
    logmsg(LOG_INFO, "clients=%d, rss=%d\n", num_clients, get_rss());
    ev_timer_again(EV_A_ timer_w);
}
