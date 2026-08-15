#include "uart.h"
#include "../hardware/io.h"

void uart_init()
{
    // Desativa todas as interrupcoes da UART temporariamente durante a config
    outb(PORT_COM1 + 1, 0x00);

    // Ativa o DLAB (Divisor Latch Access Bit), permitindo definir a velocidade do Baud Rate
    outb(PORT_COM1 + 3, 0x80);

    // Define o divisor para 115200  Baud
    // O chip base roda a 115200Hz, divisor = 115200 / Velocidade desejada
    // Para 115200 bps o divisor é 1 (0x0001)
    outb(PORT_COM1 + 0, 0x01); // Byte Baixo (DLL)
    outb(PORT_COM1 + 1, 0x00); // Byte Alto (DLM)

    // Desativa o DLAB e configura o padrao 8N1 (8 bits, Sem paridade, 1 Stop Bit)
    outb(PORT_COM1 + 3, 0x03);

    // Ativa e limpa as filas FIFO de transmissao e recepcao de hardware (limpa buffers residuais)
    outb(PORT_COM1 + 2, 0xC7);

    // Ativa os pinos DTR e RTS do circuito fisico para liberar o fluxo de hardware
    outb(PORT_COM1 + 4, 0x0B);
}

// Verifica se há algum byte esperando para ser lido no registrador de status (Linha 5)
int uart_received()
{
    // Se o bit 0 (0x01) estiver ativo, significa "Data Ready" (Dados prontos)
    return inb(PORT_COM1 + 5) & 0x01;
}

// Le um byte recebida da UART (trava a execucao ate chegar algo)
char uart_read()
{
    while (uart_received() == 0)
    {
        __asm__ volatile("hlt");
    }
    return inb(PORT_COM1);
}

// Verifica se a UART esta prronta para trnsmitir (Fila de envio vazia)
int uart_transmit_empty()
{
    return inb(PORT_COM1 + 5) & 0x20;
}

// Envia um caractere físico pela linha serial
void uart_write(char c)
{
    while (uart_transmit_empty() == 0)
    {
    };
    outb(PORT_COM1, c);
}

// Imprime uma string inteira via Serial
void uart_print(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        uart_write(str[i]);
    }
}

// Retorna o byte se existir; se não houver dados no buffer, retorna 0 para nao travar
char uart_read_nonblocking()
{
    if (uart_received())
    {
        return inb(PORT_COM1);
    }
    return 0;
}