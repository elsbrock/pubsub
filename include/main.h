
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_MAIN_H
#define PUBSUB_MAIN_H

#include "queue.h"

#define bool char
#define true 1
#define false 0

extern struct ev_loop *loop;
extern LIST_HEAD(clients_head, Client) clients;
extern int listen_fd;
extern int num_clients;

#endif
