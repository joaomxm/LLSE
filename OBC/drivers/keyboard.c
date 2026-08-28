#include "keyboard.h"
#include "video.h"
#include "../util.h"
#include "../hardware/io.h"

char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
volatile int line_ready = 0;

const char keyboard_map[128] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8', /* 0-9 */
    '9',
    '0',
    '-',
    '=',
    '\b', /* Backspace */
    '\t', /* Tab */
    'q',
    'w',
    'e',
    'r', /* 16-19 */
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', /* Enter */
    0,    /* 29   - Control */
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';', /* 30-39 */
    '\'',
    '`',
    0, /* Left shift */
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.', /* 40-49 */
    '/',
    0, /* Right shift */
    '*',
    0,   /* Alt */
    ' ', /* Space bar */
    0,   /* Caps lock */
    0,   /* 59 - F1 key ... > */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0,
    0,
    0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /*Restante fica zerado */
};

void keyboard_handler()
{
    // 1. Lê o "Scan Code" (código bruto da tecla) na porta 0x60
    unsigned char scancode = inb(0x60);

    // O chip do teclado envia um sinal quando a tecla é pressionada (bit 7 zerado)
    // e outro sinal quando ela é solta (bit 7 ativado, ou seja, valor maior que 0x80).
    // Ignora o momento de soltar a tecla por enquanto (valores maiores que 0x80).
    if (!(scancode & 0x80))
    {
        char c = keyboard_map[scancode];

        if (c == '\n')
        {
            input_buffer[input_index] = '\0'; // Finaliza a string de forma segura
            line_ready = 1;                   // Avisa o loop principal do Kernel
            input_index = 0;                  // Reseta o índice para o próximo comando
            print_string("\n");
        }
        else if (c == '\b')
        {
            if (input_index > 0)
            {
                input_index--;     // Remove do buffer lógico
                print_backspace(); // Remove da tela física
            }
        }
        else if (c != 0)
        {
            if (input_index < INPUT_BUFFER_SIZE - 1)
            {
                input_buffer[input_index++] = c; // Guarda no buffer
                print_char(c);                   // Mostra na tela
            }
        }
    }

    // 2. AVISO CRÍTICO DE FIM DE INTERRUPÇÃO (EOI - End of Interrupt)
    // Enviar o byte 0x20 para a porta do PIC Master (0x20) para notificar o PIC que a CPU nao esta processando
    outb(0x20, 0x20);
}

void read_line(char *out_buffer)
{
    line_ready = 0;
    input_index = 0;

    // Loop de espera ativa (Polling) esperando a interrupção do teclado setar 'line_ready'
    while (line_ready == 0)
    {
        __asm__ volatile("hlt"); // Coloca a CPU em modo de baixo consumo até a próxima interrupção
    }

    // Copia o resultado do buffer global para o buffer de saída do usuário
    int i = 0;
    while (input_buffer[i] != '\0')
    {
        out_buffer[i] = input_buffer[i];
        i++;
    }
    out_buffer[i] = '\0'; // Finaliza a string copiada
}

int keyboard_buffer_ready()
{
    return line_ready;
}

void get_keyboard_buffer(char *out_buffer)
{
    line_ready = 0;
    input_index = 0;

    // Copia o resultado do buffer global para o buffer de saída do usuário
    int i = 0;
    while (input_buffer[i] != '\0')
    {
        out_buffer[i] = input_buffer[i];
        i++;
    }
    out_buffer[i] = '\0'; // Finaliza a string copiada
}
