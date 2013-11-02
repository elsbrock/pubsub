
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_NET_H
#define PUBSUB_NET_H

int net_init(const char *port);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

#endif
