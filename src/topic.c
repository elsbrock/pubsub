#undef PUBSUB__FILE__
#define PUBSUB__FILE__ "topic.c"
/*
 * vim:ts=4:sw=4:expandtab
 */

#include <assert.h>
#include <string.h>

#include "data.h"

/* Given a fully-qualified topic name, (try to) walk from the root of the topic
 * tree into this topic. If the topic cannot be found and it is not going to be
 * automatically created, this method returns null. Otherwise, this method
 * either returns an existing topic reached by the path or creates a new one.
 *
 * Topics are case-sensitive and must be at least one character long. Topics
 * may include a space and must end with either '+', '#' or any other character
 * but '/'. Topics must not include the null character.
 */
Topic *walk_into_topic(char *fqdn_topic, bool autocreate) {
    /* Topic must be at least one character long. */
    /* XXX: Unicode */
    assert(strlen(fqdn_topic) > 0);
    return NULL;
}
