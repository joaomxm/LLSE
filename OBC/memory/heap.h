#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include "../util.h"

typedef struct block_header
{
    size_t size;
    int is_free;
    struct block_header *next;

} block_header_t;

void heap_init();
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif