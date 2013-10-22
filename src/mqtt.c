
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
#include "util.h"

/* Handles a CONNECT message. Assumes that the message is complete. */
int handle_connect(struct Client *client, int msg_length) {
    assert(client->state == S_CONNECTING);
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
    assert(client->inbuf[walk] == PROTOCOL_VERSION); /* protocol version */
    walk++;

    /* connect flags */
    uint8_t flags = client->inbuf[walk];

    bool has_username = (flags & 0x80) == 1 << 7;
    bool has_password = (flags & 0x40) == 1 << 6;
    bool will_retain  = (flags & 0x20) == 1 << 5;
    uint8_t will_qos  = (flags & 0x18) >> 3; /* 2 bytes */
    bool will_flag    = (flags & 0x04) == 1 << 2;
    bool clean_session = (flags & 0x02) == 1 << 1;
    /* LSB is not used */
    walk++;

    logmsg(LOG_DEBUG, "flags: %x\n", flags);
    logmsg(LOG_DEBUG, "username: %d, password: %d, will_retain: %d, will_qos: %d, will_flag: %d, clean_session: %d\n", has_username, has_password, will_retain, will_qos, will_flag, clean_session);

    /* keep alive (in seconds) */
    client->keepalive = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;
    /* XXX: register keep-alive timer */

    logmsg(LOG_DEBUG, "keepalive is %d seconds\n", client->keepalive);

    /* payload: client identifier */
    len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
    walk += 2;

    /* XXX: check if character len < 23 and bail out */
    /* XXX: check uniqueness of identifier */
    client->identifier = smalloc(sizeof(char) * len+1);
    logmsg(LOG_DEBUG, "client identitifer len: %d\n", len);
    memcpy(client->identifier, client->inbuf+walk, len);
    client->identifier[len] = '\0';
    logmsg(LOG_DEBUG, "client identifier: %s\n", client->identifier);
    walk += len;

    if (will_flag) {
        /* XXX: check length of topic and message */

        /* payload: will topic  */
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        client->will_topic = smalloc(sizeof(char) * len+1);
        memcpy(client->will_topic, client->inbuf+walk, len);
        client->will_topic[len] = '\0';
        walk += len;

        /* payload: will message */
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        client->will_msg = smalloc(sizeof(char) * len+1);
        memcpy(client->will_msg, client->inbuf+walk, len);
        client->will_msg[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "will-topic: '%s', will_msg: '%s'\n", client->will_topic, client->will_msg);
    }

    char *username = NULL;
    /* For compatibility reasons, the username flag may be set, although no
     * username is present. Therefore, check if the username really is there.
     */
    if (has_username && client->inbuf_bytes > 2+/* fixed header */walk+2) {
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        username = smalloc(sizeof(char) * len + 1);
        memcpy(username, client->inbuf+walk, len);
        username[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "username: %s\n", username);
    }

    char *password = NULL;
    /* For compatibility reasons, the password flag may be set, although no
     * password is present. Therefore, check if the password really is there.
     */
    if (has_password && client->inbuf_bytes > 2+/* fixed header */walk+2) {
        len = (client->inbuf[walk] << 8) + client->inbuf[walk+1];
        walk += 2;
        password = smalloc(sizeof(char) * len + 1);
        memcpy(password, client->inbuf+walk, len);
        password[len] = '\0';
        walk += len;

        logmsg(LOG_DEBUG, "password: %s\n", password);
    }

    /* XXX: check credentials */
    if (username != NULL)
        free(username);
    if (password != NULL)
        free(password);

    /* XXX: set state to S_CONNECTED as soon as reply is sent out */

    return 1;
}
