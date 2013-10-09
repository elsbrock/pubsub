
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_LOG_H
#define PUBSUB_LOG_H

#include <stdint.h>
#include <stdarg.h>

#define LOG_INFO    0x0f
#define LOG_WARN    0x07
#define LOG_ERR     0x03
#define LOG_DEBUG   0x01

void logmsg(uint8_t level, char *fmt, ...);

#endif
