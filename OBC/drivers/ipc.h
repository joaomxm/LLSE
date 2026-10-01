#ifndef IPC_H
#define IPC_H

#define COMMAND_BUFFER_SIZE 8
#define TELEMETRY_BUFFER_SIZE 16
#define SYSTEM_BUFFER_SIZE 8
#define ROUTER_BUFFER_SIZE 16
#define MAX_PAYLOAD_SIZE 32
#define SYNC_BYTE_1 0xAA
#define SYNC_BYTE_2 0x55
#define MAX_IPC_MODULES 16

typedef struct __attribute__((packed))
{
    unsigned char sender_id;
    unsigned char receiver_id;
    unsigned char payload_len;
    unsigned char payload_data[MAX_PAYLOAD_SIZE];
} IPC_Message;

typedef enum
{
    STATE_WAIT_START_1 = 0,
    STATE_WAIT_START_2,
    STATE_GET_SENDER,
    STATE_GET_RECEIVER,
    STATE_GET_LEN,
    STATE_GET_PAYLOAD,
    STATE_GET_CRC_1,
    STATE_GET_CRC_2
} ParserState;

typedef enum
{
    COMMAND = 0,
    TELEMETRY,
    SYSTEM,
    ROUTER,
} IPC_Modules;

typedef struct
{
    IPC_Message *message_buffer;
    int write_index;
    int read_index;
    int count;
    int capacity;
} MessageQueues;

typedef void (*ipc_handler_t)(const IPC_Message *msg);

int ipc_send(IPC_Modules module_id, const IPC_Message *message_ptr);
int ipc_receive(IPC_Modules module_id, IPC_Message *msg_buffer);

void ipc_parser(unsigned char ring_buffer_data, int tamanho);
unsigned short crc16_ccitt_false(const unsigned char *data, int length);

int ipc_register_handler(unsigned char module_id, ipc_handler_t handler);
void ipc_dispatch_all(void);

#endif