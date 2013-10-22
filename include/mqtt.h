
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_MQTT_H
#define PUBSUB_MQTT_H

#define PROTOCOL_NAME "MQIsdp"
#define PROTOCOL_VERSION 3

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

typedef union {
    unsigned char level : 2;
} qos_t;

/* Basic message. */
typedef struct {
    union {
        /* bits 0-3: msg_t */
        unsigned char type   : 4;
        unsigned char dup    : 1;
        qos_t qos    : 2;
        unsigned char retain : 1;
    } flags;
    uint8_t remaining_bytes[5];
    char *payload;
} mqtt_packet;

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

int handle_connect(struct Client *client, int msg_length);
int queue_connack(struct Client *client, conn_return_t type);
mqtt_packet *generate_packet(msg_t type, bool dup, uint8_t qos, bool retain, size_t len);
#endif
