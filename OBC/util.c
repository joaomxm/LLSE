#include "util.h"
#include <stdarg.h>

extern void print_char(char c);
extern void print_string(char *str);

// Imprime um número decimal direto na tela
void print_dec(int n)
{
    if (n == 0)
    {
        print_char('0');
        return;
    }

    if (n < 0)
    {
        print_char('-');
        n = -n;
    }

    char buffer[12];
    int i = 0;

    while (n > 0)
    {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }

    for (int j = i - 1; j >= 0; j--)
    {
        print_char(buffer[j]);
    }
}

// Imprime um número em formato Hexadecimal direto na tela
void print_hex(unsigned int n)
{
    if (n == 0)
    {
        print_char('0');
        return;
    }

    char hex_digits[] = "0123456789ABCDEF";
    char buffer[8];
    int i = 0;

    while (n > 0)
    {
        buffer[i++] = hex_digits[n % 16];
        n /= 16;
    }

    for (int j = i - 1; j >= 0; j--)
    {
        print_char(buffer[j]);
    }
}

// Printf do kernel
void printf(char *format, ...)
{
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++)
    {
        // Se encontrar um '%', olha o próximo caractere para formatação
        if (format[i] == '%')
        {
            i++; // Avança para o especificador de formato

            switch (format[i])
            {
            case 's':
            { // String
                char *s = va_arg(args, char *);
                print_string(s);
                break;
            }
            case 'd':
            { // Inteiro Decimal
                int d = va_arg(args, int);
                print_dec(d);
                break;
            }
            case 'x':
            { // Inteiro Hexadecimal
                unsigned int x = va_arg(args, unsigned int);
                print_hex(x);
                break;
            }
            case '%':
            { // Escapar o próprio símbolo de %
                print_char('%');
                break;
            }
            default: // Se for um formato desconhecido, imprime o caractere literal
                print_char('%');
                print_char(format[i]);
                break;
            }
        }
        else
        {
            // Imprime caractere normal
            print_char(format[i]);
        }
    }

    va_end(args);
}