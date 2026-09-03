#include "util.h"
#include <stdarg.h>
#include <stdint.h>

extern void print_char(char c);
extern void print_string(char *str);

// Imprime um número decimal direto na tela
void print_int_dec(int n)
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

void itoa(int num, char *str, int base)
{
    int i = 0;
    int is_negative = 0;

    // Trata o caso do número 0 explicitamente
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Trata números negativos apenas se a base for decimal
    if (num < 0 && base == 10)
    {
        is_negative = 1;
        num = -num;
    }

    // Processa cada dígito extraindo o resto da divisão
    while (num != 0)
    {
        int rem = num % base;
        // Se o dígito for maior que 9, converte para caractere 'a'-'f' (usado em Hexadecimal)
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // Adiciona o sinal de menos se for negativo
    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0'; // Finaliza a string em C

    // Como os dígitos foram colocados ao contrário (do último para o primeiro),
    // inverte a string no final
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void ftoa(float val, char *buf, int precision)
{
    // Trata valores negativos
    if (val < 0)
    {
        *buf++ = '-';
        val = -val;
    }

    // 1. Extrai a parte inteira
    int int_part = (int)val;

    // 2. Extrai a parte fracionária
    float float_part = val - (float)int_part;

    // Converte a parte inteira para string
    itoa(int_part, buf, 10);

    // Avança o ponteiro até o fim da parte inteira
    while (*buf != '\0')
    {
        buf++;
    }

    // Adiciona o ponto decimal
    if (precision > 0)
    {
        *buf++ = '.';

        // Converte a fração em um inteiro baseado na precisão
        // Exemplo: 0.35 * 100 = 35.0 -> 35
        for (int i = 0; i < precision; i++)
        {
            float_part *= 10.0f;
        }

        int frac_part = (int)(float_part + 0.5f); // +0.5f para arredondamento correto

        // Trata zeros à esquerda na fração (Ex: 12.05 não virar 12.5)
        // Adiciona suporte a digitos ajustados pela precisão
        int temp = frac_part;
        int digits = 0;
        while (temp > 0)
        {
            digits++;
            temp /= 10;
        }

        // Completa com zeros se necessário (ex: 0.05)
        for (int i = 0; i < (precision - digits); i++)
        {
            *buf++ = '0';
        }

        if (frac_part > 0)
        {
            itoa(frac_part, buf, 10);
        }
        else
        {
            *buf = '\0';
        }
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
    char buffer[64];

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
                print_int_dec(d);
                break;
            }
            case 'f':
            { // Decimal
                double f = va_arg(args, double);

                ftoa((float)f, buffer, 2);
                print_string(buffer);
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

// Copia a string de origem (src) para o destino (dest) e retorna o destino
char *strcpy(char *dest, const char *src)
{
    char *temp = dest;
    while ((*dest++ = *src++) != '\0')
        ;
    return temp;
}

// Retorna o tamanho da string
int strlen(char *str)
{

    int count = 0;
    while (str[count] != '\0')
    {
        count++;
    }
    return count;
}

// Realiza a comparacao entre strings
int strcmp(char *str1, char *str2)
{
    int str1_length = strlen(str1);
    int str2_length = strlen(str2);

    if (str1_length < str2_length)
    {
        return -1;
    }
    if (str1_length > str2_length)
    {

        return 1;
    }

    for (int i = 0; str1[i] != '\0'; i++)
    {
        if (str1[i] != str2[i])
        {
            if (str1[i] < str2[i])
                return -1;
            if (str1[i] > str2[i])
                return 1;
        }
    }
    return 0;
}

// Verifica se uma string esta em um array
int find_string_array(char *target, char **array)
{
    int array_size = sizeof(array) / sizeof(array[0]);

    for (int i = 0; i < array_size; i++)
    {
        if (strcmp(target, array[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}