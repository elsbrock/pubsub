
/*
 * vim:ts=4:sw=4:expandtab
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "main.h"

#define LOGBUF 4096

static uint8_t log_level = LOG_DEBUG;
static bool verbose = false;

static void _logmsg(const uint8_t level, const bool print, const char *fmt, va_list args);
void logmsg(const uint8_t level, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _logmsg(level, verbose, fmt, args);
    va_end(args);
}

static void _logmsg(const uint8_t level, const bool print, const char *fmt, va_list args) {
    FILE *stream = stdout;

    if ((level & log_level) != log_level)
        return;

    static char *levelstr;
    if (0) {
    } else if ((level & LOG_INFO) == LOG_INFO) {
        levelstr = "INFO  - ";
    } else if ((level & LOG_WARN) == LOG_WARN) {
        levelstr = "WARN  - ";
        stream = stderr;
    } else if ((level & LOG_ERR) == LOG_ERR) {
        levelstr = "ERROR - ";
        stream = stderr;
    } else if ((level & LOG_DEBUG) == LOG_DEBUG) {
        levelstr = "DEBUG - ";
        stream = stderr;
    }

    size_t levelstrlen = strlen(levelstr);

    time_t t = time(NULL);
    struct tm result;
    struct tm *tmp = localtime_r(&t, &result);

    char message[LOGBUF];
    size_t len = strftime(message, sizeof(message), "%x %X ", tmp);

    assert(LOGBUF > len + levelstrlen);
    memcpy(message+len, levelstr, levelstrlen+1);

    fprintf(stream, "%s", message);
    vfprintf(stream, fmt, args);
}
