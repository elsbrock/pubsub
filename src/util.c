
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "log.h"

void *smalloc(size_t size) {
    void *result = malloc(size);
    if (result == NULL) {
        logmsg(LOG_ERR, "malloc(%zu) failed\n", size);
        exit(1);
    }
    return result;
}

void *scalloc(size_t size) {
    void *result = calloc(size, 1);
    if (result == NULL) {
        logmsg(LOG_ERR, "malloc(%zu) failed\n", size);
        exit(1);
    }
    return result;
}

void *srealloc(void *p, size_t size) {
    void *result = realloc(p, size);
    if (result == NULL) {
        logmsg(LOG_ERR, "realloc(%zu) failed\n", size);
        exit(1);
    }
    return result;
}

void hexdump(const char *desc, const void *addr, size_t len) {
    size_t i;
    unsigned char buff[17];
    const unsigned char *pc = addr;

    if (desc != NULL)
        printf ("%s:\n", desc);

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n", buff);
            printf ("  %04zx ", i);
        }

        printf (" %02x", pc[i]);
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }
    printf ("  %s\n", buff);
}

size_t get_rss() {
    long rss = 0;
    FILE* file = NULL;
    if ((file = fopen("/proc/self/statm", "r")) == NULL)
        return 0;
    if (fscanf(file, "%*s%ld", &rss) != 1 ) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);
}
