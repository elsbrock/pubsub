
/*
 * vim:ts=4:sw=4:expandtab
 */
#ifndef PUBSUB_UTIL_H
#define PUBSUB_UTIL_H

void *smalloc(size_t size);
void *scalloc(size_t size);
void hexdump(char *desc, void *addr, int len);
size_t get_rss();

#endif
