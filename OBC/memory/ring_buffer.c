#include "ring_buffer.h"
#include "../util.h"

static RingBuffer ring_buffer;

unsigned char buffer[BUFFER_SIZE];

void ring_buffer_init()
{
    ring_buffer.write_index = 0;
    ring_buffer.read_index = 0;
    ring_buffer.count = 0;
    ring_buffer.r_buffer = buffer;
}

void ring_buffer_put(char data)
{

    if (ring_buffer.count == BUFFER_SIZE)
    {
        return;
    }

    ring_buffer.r_buffer[ring_buffer.write_index] = data;
    ring_buffer.write_index = (ring_buffer.write_index + 1) & (BUFFER_SIZE - 1);
    ring_buffer.count = ring_buffer.count + 1;
}

char ring_buffer_get()
{
    if (ring_buffer.count == 0)
    {
        return 0;
    }

    char data = ring_buffer.r_buffer[ring_buffer.read_index];
    ring_buffer.read_index = (ring_buffer.read_index + 1) % BUFFER_SIZE;
    ring_buffer.count = ring_buffer.count - 1;

    return data;
}

int ring_buffer_empty()
{

    if (ring_buffer.count == 0)
    {
        return 1;
    }

    return 0;
}