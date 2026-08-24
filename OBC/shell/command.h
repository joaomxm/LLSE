#ifndef COMMAND_H
#define COMMAND_H
#include "../util.h"

typedef void (*command_func_t)(int argc, char **args);

typedef struct
{
    const char *name;
    command_func_t func;
    const char *help;
} command_entry_t;

void cmd_ping(int argc, char **args);

void command_execute(char **args);

#endif