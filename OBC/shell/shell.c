#include "shell.h"
#include "../util.h"
#include "../drivers/keyboard.h"
#include "../drivers/video.h"
#include "../memory/heap.h"
#include "../drivers/ipc.h"

#include "command.h"

// Realiza a leitura de um comando
void read_command()
{
    char *str_command = (char *)kmalloc(256); // 64 elementos

    get_keyboard_buffer(str_command);

    char **args = parse(str_command);

    kfree(str_command);
    int array_size = sizeof(args);

    if (args != NULL)
    {
        command_execute(args);
    }

    kfree(args);

    printf("OS_Kernel> ");
}

// Realiza o parse da string do comando
char **parse(char *str_command)
{
    int count_args = 0;

    char **args = (char **)kmalloc(64); // 16 elementos

    if (args != NULL)
    {
        for (int i = 0; i < 16; i++)
        {
            args[i] = NULL;
        }
    }

    for (int i = 0; str_command[i] != '\0'; i++)
    {
        if (i == 0 && str_command[i] != 0x20)
        {
            args[count_args] = &str_command[i];
            count_args++;
            continue;
        }

        if (str_command[i] == 0x20)
        {
            if (str_command[i + 1] != '\0' && str_command[i + 1] != 0x20)
            {
                args[count_args] = &str_command[i + 1];
                count_args++;
            }
            str_command[i] = '\0';
        }
    }

    return args;
}

void shell_ipc_handler(const IPC_Message *msg)
{
    printf("\n[SHELL IPC] Recebido %d bytes do modulo %d\n",
           msg->payload_len, msg->sender_id);

    // Teste do comando recebido
    char *str_command = (char *)kmalloc(256);

    for (int i = 0; i < msg->payload_len; i++)
    {
        str_command[i] = msg->payload_data[i];
    }

    char **args = parse(str_command);

    kfree(str_command);
    int array_size = sizeof(args);

    if (args != NULL)
    {
        command_execute(args);
    }

    kfree(args);
}

void shell_init(void)
{
    ipc_register_handler(0x00, shell_ipc_handler);
}
