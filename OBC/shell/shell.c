#include "shell.h"
#include "../util.h"
#include "../drivers/keyboard.h"
#include "../drivers/video.h"
#include "../memory/heap.h"

#include "command.h"

// Realiza a leitura de um comando
void read_command()
{
    char *str_command = (char *)kmalloc(256); // 64 elementos

    printf("OS_Kernel> ");
    read_line(str_command);

    char **args = parse(str_command);

    kfree(str_command);
    int array_size = sizeof(args);

    if (args != NULL)
    {
        command_execute(args);
    }

    kfree(args);
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
