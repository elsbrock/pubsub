
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_NET_H
#define PUBSUB_NET_H

void client_read_cb(EV_P_ struct ev_io *peer_w, int revents);
void client_write_cb(EV_P_ struct ev_io *peer_w, int revents);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

#endif
