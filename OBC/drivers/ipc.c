#include "ipc.h"
#include "../util.h"
#include "video.h"

static IPC_Message command_buffer[COMMAND_BUFFER_SIZE];
static IPC_Message telemetry_buffer[TELEMETRY_BUFFER_SIZE];
static IPC_Message system_buffer[SYSTEM_BUFFER_SIZE];
static IPC_Message router_buffer[ROUTER_BUFFER_SIZE];

static MessageQueues message_queues[4];

static ParserState current_state = STATE_WAIT_START_1;
static IPC_Message temp_msg;
static unsigned char payload_bytes_read = 0;
static unsigned short received_crc = 0;
static unsigned char payload_idx = 0;

static ipc_handler_t ipc_handlers[MAX_IPC_MODULES];

unsigned short crc16_ccitt_false(const unsigned char *data, int length)
{
    uint16_t crc = 0xFFFF;
    int i, j;
    for (i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
        crc &= 0xFFFF;
    }
    return crc;
}

void ipc_init()
{

    for (int i = 0; i < MAX_IPC_MODULES; i++)
    {
        ipc_handlers[i] = NULL;
    }

    message_queues[0].write_index = 0;
    message_queues[0].read_index = 0;
    message_queues[0].count = 0;
    message_queues[0].message_buffer = command_buffer;
    message_queues[0].capacity = COMMAND_BUFFER_SIZE;

    message_queues[1].write_index = 0;
    message_queues[1].read_index = 0;
    message_queues[1].count = 0;
    message_queues[1].message_buffer = telemetry_buffer;
    message_queues[1].capacity = TELEMETRY_BUFFER_SIZE;

    message_queues[2].write_index = 0;
    message_queues[2].read_index = 0;
    message_queues[2].count = 0;
    message_queues[2].message_buffer = system_buffer;
    message_queues[2].capacity = SYSTEM_BUFFER_SIZE;

    message_queues[3].write_index = 0;
    message_queues[3].read_index = 0;
    message_queues[3].count = 0;
    message_queues[3].message_buffer = router_buffer;
    message_queues[3].capacity = ROUTER_BUFFER_SIZE;
}

int ipc_register_handler(unsigned char module_id, ipc_handler_t handler)
{
    if (module_id >= MAX_IPC_MODULES || handler == NULL)
    {
        return 0;
    }

    ipc_handlers[module_id] = handler;
    return 1;
}

void ipc_dispatch_all(void)
{
    IPC_Message msg;

    for (unsigned char mod_id = 0; mod_id < MAX_IPC_MODULES; mod_id++)
    {
        if (ipc_handlers[mod_id] != NULL)
        {
            while (ipc_receive(mod_id, &msg))
            {
                ipc_handlers[mod_id](&msg);
            }
        }
    }
}

int ipc_send(IPC_Modules module_id, const IPC_Message *message_ptr)
{

    printf("Vou enviar os dados, modulo: %d - payload %x ", module_id, message_ptr->payload_data);

    MessageQueues *message_queue = &message_queues[module_id];

    if (message_queue->count == message_queue->capacity)
    {
        return 0;
    }

    message_queue->message_buffer[message_queue->write_index] = *message_ptr;
    message_queue->write_index = (message_queue->write_index + 1) & (message_queue->capacity - 1);
    message_queue->count = message_queue->count + 1;

    return 1;
}

int ipc_receive(IPC_Modules module_id, IPC_Message *msg_buffer)
{

    MessageQueues *message_queue = &message_queues[module_id];

    if (message_queue->count == 0)
    {
        return 0;
    }

    IPC_Message data = message_queue->message_buffer[message_queue->read_index];
    message_queue->read_index = (message_queue->read_index + 1) & (message_queue->capacity - 1);
    message_queue->count = message_queue->count - 1;

    *msg_buffer = data;

    return 1;
}

void ipc_parser(unsigned char ring_buffer_data, int tamanho)
{
    switch (current_state)
    {
    case STATE_WAIT_START_1:
        if (ring_buffer_data == SYNC_BYTE_1)
        {
            temp_msg.sender_id = 0;
            temp_msg.receiver_id = 0;
            temp_msg.payload_len = 0;
            payload_idx = 0;
            received_crc = 0;

            current_state = STATE_WAIT_START_2;
        }
        break;

    case STATE_WAIT_START_2:
        if (ring_buffer_data == SYNC_BYTE_2)
        {
            current_state = STATE_GET_SENDER;
        }
        else
        {
            current_state = STATE_WAIT_START_1;
        }
        break;

    case STATE_GET_SENDER:
        temp_msg.sender_id = ring_buffer_data;
        current_state = STATE_GET_RECEIVER;
        break;

    case STATE_GET_RECEIVER:

        temp_msg.receiver_id = ring_buffer_data;
        current_state = STATE_GET_LEN;
        break;

    case STATE_GET_LEN:
        if (ring_buffer_data <= MAX_PAYLOAD_SIZE)
        {
            temp_msg.payload_len = ring_buffer_data;
            payload_idx = 0;

            if (temp_msg.payload_len == 0)
            {
                current_state = STATE_GET_CRC_1;
            }
            else
            {
                current_state = STATE_GET_PAYLOAD;
            }
        }
        else
        {
            current_state = STATE_WAIT_START_1;
        }
        break;

    case STATE_GET_PAYLOAD:

        temp_msg.payload_data[payload_idx] = ring_buffer_data;
        payload_idx++;
        if (payload_idx >= temp_msg.payload_len)
        {
            current_state = STATE_GET_CRC_1;
        }
        break;

    case STATE_GET_CRC_1:
        received_crc = 0;
        received_crc = ring_buffer_data;
        current_state = STATE_GET_CRC_2;
        break;

    case STATE_GET_CRC_2:
        received_crc = ((unsigned short)received_crc << 8) | ring_buffer_data;

        static unsigned char test_buffer[35];

        test_buffer[0] = temp_msg.sender_id;
        test_buffer[1] = temp_msg.receiver_id;
        test_buffer[2] = temp_msg.payload_len;

        for (unsigned char i = 0; i < temp_msg.payload_len; i++)
        {
            test_buffer[3 + i] = temp_msg.payload_data[i];
        }

        unsigned short total_length = 3 + (unsigned short)temp_msg.payload_len;

        unsigned short computed_crc = crc16_ccitt_false(test_buffer, total_length);

        printf("Calculado: ");
        print_hex((unsigned char)(computed_crc >> 8));
        print_hex((unsigned char)(computed_crc & 0xFF));
        printf("\n");

        printf("Recebido:  ");
        print_hex((unsigned char)(received_crc >> 8));
        print_hex((unsigned char)(received_crc & 0xFF));
        printf("\n");

        if (computed_crc == received_crc)
        {
            printf("=> CRC OK!\n");
            ipc_send((IPC_Modules)temp_msg.receiver_id, &temp_msg);
        }
        else
        {
            printf("=> CRC INVALIDO!\n");
        }

        current_state = STATE_WAIT_START_1;
        break;
    }
}
