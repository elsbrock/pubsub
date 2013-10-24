
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_DATA_H
#define PUBSUB_DATA_H

#include <time.h>
#include <ev.h>

#include "main.h"
#include "queue.h"
#include "mqtt.h"

#define BUF_LEN 4096

struct Context {
    time_t started_at;
    int num_clients;
};

/* An envelope is an MQTT message that is placed in the outgoing queue of a
 * client and about to be sent. */
struct Envelope {
    /* The time at which the message was enqueued. */
    time_t enqueued_at;

    /* The total length of the message, and the number of bytes that have been
     * already submitted. */
    size_t bytes_total;
    size_t bytes_sent;

    /* The actual message, consisting of a fixed and a variable header which is
     * optional though. */
    char *msg;

    /* An envelope is part of the outgoing messages queue of the client. */
    LIST_ENTRY(Envelope) entries;
};

struct Client {
    int fd;

    /* The read watcher is started as soon as the connection is accepted. It is
     * never stopped as long as the client is connected. */
    struct ev_io *read_w;

    /* The write watcher is started as soon as a message has been enqueued. If
     * there is no message left in the queue, the write_watcher is stopped to
     * avoid busy-looping. */
    struct ev_io *write_w;

    /* Each client is embedded into the clients list. */
    LIST_ENTRY(Client) entries;

    char *identifier;
    uint16_t keepalive;
    char *will_topic;
    char *will_msg;

    enum { S_CONNECTING = 0, S_CONNECTED = 1} state;

    time_t connected_at;
    time_t last_ping;

    /* Used to store incoming messages. */
    char *inbuf;
    int inbuf_bytes;

    /* Used to store outgoing messages. */
    int outgoing_num;
    LIST_HEAD(outgoing_head, Envelope) outgoing_msgs;
};

/* A topic is one element of the tree data structure that represents the
 * hierarchical namespace of MQTT. Each topic may have a number of subscribing
 * clients that will be informed about new messages that have been published to
 * the topic by other clients. */
struct Topic {
    char *name;
    msg_t retained_msg;

    /* Used to store subscriptions. */
    LIST_HEAD(subtopic_head, Topic) subtopics;
    LIST_ENTRY(Topic) entries;

    /* Used to store subscriptions. */
    int subscriptions_num;
    LIST_HEAD(subscription_head, Subscription) subscriptions;
};

struct Subscription {
    /* The type of subscription: PLAIN means a subscription with a
     * fully-qualified path without any wildcards; SINGLE means a subscription
     * with a single-level wildcards, MULTI a subscription with a multi-level
     * wildcard. */
    enum { S_PLAIN, S_SINGLE, S_MULTI } type;

    /* The subscribing client. */
    Client *client;
};

#endif
