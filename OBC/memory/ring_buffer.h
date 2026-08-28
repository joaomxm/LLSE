#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define BUFFER_SIZE 2

typedef struct
{
    char *r_buffer;
    int write_index;
    int read_index;
    int count;
} RingBuffer;

void ring_buffer_init();
void ring_buffer_put(char data);
char ring_buffer_get();
int ring_buffer_empty();

#endif