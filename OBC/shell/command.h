#ifndef COMMAND_H
#define COMMAND_H
#include "../util.h"
#include "kernel_fsm.h"

typedef void (*command_func_t)(int argc, char **args);

typedef struct
{
    const char *name;
    command_func_t func;
    const char *help;
} command_entry_t;

typedef enum
{
    NADIR,
    SUN_TRACKING,
    SPIN,
} Orientation_Mode;

typedef enum
{
    STANDBY,
    ACTIVE,
} Magnetorquers_Status;

typedef struct
{
    float bus_voltage;
    float solar_array_current;
    float battery_temperature;
} Eletrical_Power_System;

typedef struct
{
    Orientation_Mode orientation_mode;
    float gyro_x;
    float gyro_y;
    float gyros_z;
    Magnetorquers_Status magnetorquers_status;
} ADCS;

typedef struct
{
    char *uptime;
    float cpu_load;
    int usage_heap_memory;
    int total_heap_memory;
    int boot_count;
    Operation_Mode mode;
} OBC;

typedef struct
{
    float cpu_temperature;
    float transceptor_rf_temperature;
} Thermal_Subsystem;

static const char *Orientation_Mode_text[] = {
    "NADIR",
    "SUN_TRACKING",
    "SPIN",
};
static const char *Magnetorquers_Status_text[] = {
    "STANDBY",
    "ACTIVE",

};

void cmd_ping(int argc, char **args);
void cmd_clear();
void cmd_help();
void cmd_stat();
void command_execute(char **args);

#endif