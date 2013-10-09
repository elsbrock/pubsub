
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_DATA_H
#define PUBSUB_DATA_H

#include <time.h>
#include "queue.h"

#define BUF_LEN 4096

struct Context {
    time_t started_at;
    int num_clients;
};

struct Client {
    int peer_fd;
    char *inbuf;
    int inbuf_bytes;
    /* outbuf: should be a queue with enqueued msgs */
    time_t connected_at;
    enum { S_CONNECTING = 0, S_CONNECTED = 1} state;
    LIST_ENTRY(Client) entries;
};

#endif
