#include "../drivers/video.h"
#include "io.h"

static volatile unsigned int timer_ticks = 0;

void timer_handler()
{
    timer_ticks++;

    // Endereço linear: (0 * 80 + 79) * 2 = 158 - para adicionar caractere para animacao
    char *video_memory = (char *)VIDEO_ADDRESS;

    // Adiciona animação simples usando os ticks (exibe '/' ou '-')
    if (timer_ticks & 0x20)
    {
        video_memory[158] = '/';
        video_memory[159] = 0x0E; // Texto amarelo
    }
    else
    {
        video_memory[158] = '-';
        video_memory[159] = 0x0E;
    }

    // Notifica o PIC Master que a interrupcao do relogio acabou
    outb(0x20, 0x20);
}

unsigned int get_timer_ticks()
{
    return timer_ticks;
}