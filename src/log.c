
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

static uint8_t log_level = LOG_DEBUG;

static const char *levelstr(uint8_t level) {
    switch (level) {
        case LOG_INFO:
            return "INFO";
        case LOG_WARN:
            return "WARN";
        case LOG_ERR:
            return "ERROR";
        case LOG_DEBUG:
            return "DEBUG";
        default:
            return "";
    };
}

void logmsg(uint8_t level, const char *fmt, ...) {
    FILE *stream = (level == LOG_INFO) ? stdout : stderr;
    va_list args;
    char timestamp[64];

    if ((level & log_level) != log_level)
        return;

    time_t now = time(NULL);
    struct tm now_tm;
    localtime_r(&now, &now_tm);
    strftime(timestamp, sizeof(timestamp), "%x %X", &now_tm);

    fprintf(stream, "%s %-5s - ", timestamp, levelstr(level));
    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
}
