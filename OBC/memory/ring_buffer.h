#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define BUFFER_SIZE 16

typedef struct
{
    unsigned char *r_buffer;
    unsigned char write_index;
    unsigned char read_index;
    unsigned char count;
} RingBuffer;

void ring_buffer_init();
void ring_buffer_put(unsigned char data);
unsigned char ring_buffer_get();
int ring_buffer_empty();

#endif