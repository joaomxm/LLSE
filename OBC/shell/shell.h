#ifndef SHELL_H
#define SHELL_H

void read_command();
char **parse(char *command);

typedef struct
{
    char *name;
    void (*func)(char **args);
} Command;

#endif