
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_MQTT_H
#define PUBSUB_MQTT_H

#define PROTOCOL_NAME "MQIsdp"
#define PROTOCOL_VERSION 3

#include "data.h"

typedef struct Client Client;
typedef struct Envelope Envelope;
typedef struct mqtt_msg mqtt_msg;

typedef enum {
    /* RESERVED: 0x0 */
    T_CONNECT = 0x1 << 4,
    T_CONNACK = 0x2 << 4,
    T_PUBLISH = 0x3 << 4,
    T_PUBACK  = 0x4 << 4,
    T_PUBREC  = 0x5 << 4,
    T_PUBREL  = 0x6 << 4,
    T_PUBCOMP = 0x7 << 4,
    T_SUBSCRIBE = 0x8 << 4,
    T_SUBACK    = 0x9 << 4,
    T_UNSUBSCRIBE = 0x10 << 4,
    T_UNSUBACK    = 0x11 << 4,
    T_PINGREQ  = 0x12 << 4,
    T_PINGRESP = 0x13 << 4,
    T_DISCONNECT = 0x14 << 4
    /* RESERVED: 0x15 */
} msg_t;

/* Basic message. */
struct mqtt_msg {
    msg_t type;

    struct {
        bool duplicate;
        uint8_t qos;
        bool retain;
    } *flags;

    /* The number of bytes the remaining length occupies. */
    uint8_t remaining_num;

    /* Remaining length can take up to 4 bytes. */
    uint8_t  remaining_bytes[4];
    uint32_t payload_len;

    /* The payload (optional) */
    char *payload;
};

/* The possible CONNECT return codes used in a CONNACK. */
typedef enum {
    R_ACK           = 0x00,

    /* refused: unacceptable protocol version */
    R_NAK_PROTO     = 0x01,
    /* refused: identifier rejected */
    R_NAK_IDREJECT  = 0x02,
    /* refused: server unavailable */
    R_NAK_UNAVAIL   = 0x03,
    /* refused: bad user name or password */
    R_NAK_AUTHFAIL  = 0x04,
    /* refused: not authorized */
    R_NAK_NOAUTH    = 0x05

    /* reserved: 0x06-0xFF */
} conn_return_t;

int handle_connect(Client *client, size_t msg_length);
int create_msg(mqtt_msg *msg, msg_t type, uint8_t qos, bool retain, size_t len);
int enqueue_msg(Client *client, mqtt_msg *msg);
#endif
