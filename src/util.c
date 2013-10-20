
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>

#include "log.h"

void *smalloc(size_t size) {
    void *result = malloc(size);
    if (result == NULL) {
        logmsg(LOG_ERR, "malloc(%d) failed\n", size);
        exit(1);
    }
    return result;
}
