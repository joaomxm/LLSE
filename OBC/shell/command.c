#include "command.h"
#include "../drivers/video.h"
#include "../drivers/uart.h"
#include "../memory/heap.h"

void cmd_ping(int argc, char **args)
{
    printf("[OBC] PONG! Satelite online e operando.\n");
    uart_print("\r\n[OBC] PONG! Satelite online e operando.\r\n");
}

static command_entry_t command_table[];
static int get_command_count();

static command_entry_t command_table[] = {
    {"ping", cmd_ping, "Verifica conectividade com o computador de bordo"},
};

static int get_command_count()
{
    return sizeof(command_table) / sizeof(command_entry_t);
}

void command_execute(char **args)
{
    if (args == NULL || args[0] == NULL)
    {
        return;
    }

    int argc = 0;

    while (args[argc] != NULL)
    {
        argc++;
    }

    int total_commands = get_command_count();

    for (int i = 0; i < total_commands; i++)
    {
        if (strcmp(args[0], command_table[i].name) == 0)
        {
            command_table[i].func(argc, args);
            return;
        }
    }

    printf("[ERRO] Comando desconhecido: '%s'. Digite HELP.\n", args[0]);
    uart_print("\r\n[OBC ERRO] Comando desconhecido.\r\n");
}