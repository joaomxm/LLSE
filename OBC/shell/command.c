#include "command.h"
#include "../drivers/video.h"
#include "../drivers/uart.h"
#include "../memory/heap.h"

static command_entry_t command_table[];
static int get_command_count();

static command_entry_t command_table[] = {
    {"ping", cmd_ping, "Verifica conectividade com o computador de bordo"},
    {"clear", cmd_clear, "Realiza a limpeza do terminal"},
    {"help", cmd_help, "Detalhes dos comandos disponiveis"}};

void cmd_ping(int argc, char **args)
{
    printf("[OBC] PONG! Satelite online e operando.\n");
    uart_print("\r\n[OBC] PONG! Satelite online e operando.\r\n");
}

void cmd_clear()
{
    clear_screen();
}

void cmd_help()
{
    int num_commands = get_command_count();

    printf("Comandos disponiveis:\n");
    for (int i = 0; i < num_commands; i++)
    {
        printf("'%s' - (%s)\n", command_table[i].name, command_table[i].help);
    }
}

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