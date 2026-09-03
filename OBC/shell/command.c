#include "command.h"
#include "../drivers/video.h"
#include "../drivers/uart.h"
#include "../memory/heap.h"

static command_entry_t command_table[];
static int get_command_count();

static command_entry_t command_table[] = {
    {"ping", cmd_ping, "Verifica conectividade com o computador de bordo"},
    {"clear", cmd_clear, "Realiza a limpeza do terminal"},
    {"help", cmd_help, "Detalhes dos comandos disponiveis"},
    {"stat", cmd_stat, "Telemetria"},
};

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

void cmd_stat()
{
    Eletrical_Power_System eletrical_power_system = {
        +12.4, // V
        +1.8,  // A
        +18.5, // Celsius
    };

    ADCS adcs = {
        NADIR,
        0.01,  // X: rad/s,
        -0.02, // Y: rad/s,
        0.00,  // Z: rad/s
        STANDBY,
    };

    OBC obc = {
        "00h 42m 15s",
        12.5,  // Percentage
        16384, // bytes
        65536, // bytes
        3,
        NOMINAL_MODE,
    };

    Thermal_Subsystem thermal_subsystem = {
        +35.2, // Celsius
        +28.0, // Celsius};
    };

    printf("=============================================\n");
    printf("[OBC TELEMETRY REPORT - SAT_UNIT_01]\n");
    printf("=============================================\n");
    printf("[SYS] Uptime         : %s\n", obc.uptime);
    printf("[SYS] Mode           : %s\n", Operation_Mode_text[obc.mode]);
    printf("[SYS] Boot Count     : %d\n", obc.boot_count);
    printf("---------------------------------------------\n");
    printf("[EPS] Bus Voltage    : %f V\n", eletrical_power_system.bus_voltage);
    printf("[EPS] Solar Current  : %f A\n", eletrical_power_system.solar_array_current);
    printf("[EPS] Bat Temp       : %f C\n", eletrical_power_system.battery_temperature);
    printf("---------------------------------------------\n");
    printf("[ADCS] Orientation   : %s\n", Orientation_Mode_text[adcs.orientation_mode]);
    printf("[ADCS] Gyro X/Y/Z    : %f / %f / %f\n", adcs.gyro_x, adcs.gyro_y, adcs.gyros_z);
    printf("[ADCS] Magnetorquers : %s\n", Magnetorquers_Status_text[adcs.magnetorquers_status]);
    printf("---------------------------------------------\n");
    printf("[THERMAL] OBC Temp   : %f C\n", thermal_subsystem.cpu_temperature);
    printf("[THERMAL] RF Temp    : %f C\n", thermal_subsystem.transceptor_rf_temperature);
    printf("=============================================\n");
    printf("[STATUS: ALL SUBSYSTEMS NOMINAL]\n");
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