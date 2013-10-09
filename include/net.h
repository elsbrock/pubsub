
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_NET_H
#define PUBSUB_NET_H

void peer_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

enum {
    /* RESERVED: 0x0 */
    T_CONNECT = 0x1 << 4,
    T_CONNACK = 0x2 << 4,
    T_PUBLISH = 0x3 << 4,
    T_PUBACK  = 0x4 << 4,
    T_PUBREC  = 0x5 << 4,
    T_PUBREL  = 0x6 << 4,
    T_PUBCOMP = 0x7 << 4,
    T_SUBSCRIBE = 0x8 << 4,
    T_SUBACK = 0x9 << 4,
    T_UNSUBSCRIBE = 0x10 << 4,
    T_UNSUBACK = 0x11 << 4,
    T_PINGREQ = 0x12 << 4,
    T_PINGRESP = 0x13 << 4,
    T_DISCONNECT = 0x14 << 4,
    /* RESERVED: 0x15 */
} msg_type;

#endif
