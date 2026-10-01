#ifndef SHELL_H
#define SHELL_H

#include "../drivers/ipc.h"

void read_command();
char **parse(char *command);

typedef struct
{
    char *name;
    void (*func)(char **args);
} Command;

void shell_init(void);
void shell_ipc_handler(const IPC_Message *msg);

#endif