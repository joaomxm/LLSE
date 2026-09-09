#ifndef KERNEL_FSM
#define KERNEL_FSM

typedef enum
{
    BOOT_MODE,
    NOMINAL_MODE, //: Operação padrão; todos os comandos respondem normalmente.
    SAFE_MODE,    //: Desativa comandos pesados, mantém apenas a comunicação básica ligada (modo de economia de energia).
    PAYLOAD_MODE, //: Modo de coleta/execução de tarefas pesadas.
} Operation_Mode;

static const char *Operation_Mode_text[] = {
    "BOOT_MODE",
    "NOMINAL_MODE",
    "SAFE_MODE",
    "PAYLOAD_MODE",
};

void init_kernel_operation_mode_fsm();
int set_current_operation_mode_fsm(Operation_Mode new_operation_mode);
Operation_Mode get_current_operation_mode_fsm();

#endif