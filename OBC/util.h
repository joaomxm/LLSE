#ifndef UTIL_H
#define UTIL_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

void print_hex(unsigned int n);
void print_int_dec(int n);
void printf(char *format, ...);
char *strcpy(char *dest, const char *src);
int strlen(char *str);
int strcmp(char *str1, char *str2);
int find_string_array(char *target, char **array);
void itoa(int num, char *str, int base);
void ftoa(float val, char *buf, int precision);

#endif