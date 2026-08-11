#ifndef IO_H
#define IO_H

/*
Função que envia um byte para a porta de de hardware (I/O)
outb %0, %1 -> intrucao de maquina para enviar o dado %0 para a porta %1
 : : "a"(data), "Nd"(port) ->coloca o conteudo de data no registrado AL e o numero da porta no regeistrador de hardware
 */

static inline __attribute__((always_inline)) void outb(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Função que escuta e retorna um byte que o chip de hardware deicou em uma porta especifica.
static inline __attribute__((always_inline)) unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

#endif