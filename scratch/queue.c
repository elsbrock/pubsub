#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
 
struct foo {
    int a, b, c;
    LIST_ENTRY(foo) pointers;
};
 
LIST_HEAD(foo_list, foo);
 
int main(void)
{
    LIST_HEAD(foo_list, foo) head;
     
    LIST_INIT(&head);
     
    struct foo *item = malloc(sizeof(struct foo));
    item->a = 69;
    item->b = 123;
    LIST_INSERT_HEAD(&head, item, pointers);
     
    LIST_FOREACH(item, &head, pointers)
    {
        printf("a = %d b = %d\n", item->a, item->b);
    }
     
    while (!LIST_EMPTY(&head))
    {
        item = LIST_FIRST(&head);
        LIST_REMOVE(item, pointers);
        free(item);
    }
     
    return (0);
}
