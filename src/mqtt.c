
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "log.h"
#include "data.h"
#include "mqtt.h"

/* Handles a CONNECT message. Assumes that the message is complete. */
int handle_connect(struct Client *client, int msg_length) {
    assert((client->inbuf[0] & 0xF0) >> 4 == 1); 
    assert((client->inbuf[0] & 0xF) == 0); /* DUP, QoS & Retain */

    int len, walk = 2;

    /* protocol identifier */
    len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;

    /* XXX: change these to IFs and return appropriate MSG to client */
    assert(len == 6);
    assert(strncmp(client->inbuf+walk, PROTOCOL_NAME, len) == 0);
    walk += len;
    assert(client->inbuf[walk] == 3); /* protocol version */
    walk++;

    /* connect flags */
    uint8_t flags = client->inbuf[walk];

    bool has_username = (flags & 0x80) == 1 << 7;
    bool has_password = (flags & 0x40) == 1 << 6;
    bool will_retain  = (flags & 0x20) == 1 << 5;
    uint8_t will_qos  = (flags & 0x18) == 1 << 4; /* 2 bytes */
    bool will_flag    = (flags & 0x04) == 1 << 2;
    bool clean_session = (flags & 0x02) == 1 << 1;
    /* LSB is not used */
    walk++;

    logmsg(LOG_DEBUG, "flags: %x\n", flags);
    logmsg(LOG_DEBUG, "username: %d, password: %d, will_retain: %d, will_qos: %d, will_flag: %d, clean_session: %d\n", has_username, has_password, will_retain, will_qos, will_flag, clean_session);

    /* keep alive (in seconds) */
    client->keepalive = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;

    logmsg(LOG_DEBUG, "keepalive is %d seconds\n", client->keepalive);

    return 0;
}
