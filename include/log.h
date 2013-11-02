
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

#define log_info(s, ...)     logmsg(LOG_INFO, s, __VA_ARGS__)
#define log_warn(s, ...)     logmsg(LOG_WARN, s, __VA_ARGS__)
#define log_err(s, ...)      logmsg(LOG_ERR, s, __VA_ARGS__)
#define log_debug(s, ...)    logmsg(LOG_DEBUG, s, __VA_ARGS__)

void logmsg(uint8_t level, char *fmt, ...);

#endif
