#ifndef VIDEO_H
#define VIDEO_H

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Definicoes do tamanho da tela -> 25 colunas por 80 linhas = 2000
#define VIDEO_ADDRESS 0xB8000 // Endereço inicial de video

void clear_screen();
void print_char(char c);
void print_string(const char *str);
void print_backspace();

extern int cursor_position;

extern int cursor_x;
extern int cursor_y;

#endif