
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_NET_H
#define PUBSUB_NET_H

void peer_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

#endif
