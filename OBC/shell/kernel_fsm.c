#include "kernel_fsm.h"
#include "../util.h"

static Operation_Mode current_operation_mode;

void init_kernel_operation_mode_fsm()
{
    current_operation_mode = 0;
}

int set_current_operation_mode_fsm(Operation_Mode new_operation_mode)
{
    // TODO: Adicionar novas validacoes
    if (current_operation_mode == 0 && new_operation_mode == 3)
    {
        printf("Nao e possivel alterar o modo de operacao");
        return 0;
    }

    current_operation_mode = new_operation_mode;
    return 1;
}

Operation_Mode get_current_operation_mode_fsm()
{
    return current_operation_mode;
}