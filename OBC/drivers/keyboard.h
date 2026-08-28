#ifndef KEYBOARD_H
#define KEYBOARD_H

#define INPUT_BUFFER_SIZE 256
void keyboard_handler();
void read_line(char *out_buffer);
int keyboard_buffer_ready();
void get_keyboard_buffer(char *out_buffer);
#endif