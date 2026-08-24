#include "drivers/video.h"
#include "drivers/keyboard.h"
#include "drivers/uart.h"
#include "hardware/idt.h"
#include "hardware/timer.h"
#include "memory/pmm.h"
#include "memory/paging.h"
#include "util.h"
#include "memory/heap.h"

extern void keyboard_handler_wrapper();
extern void timer_handler_wrapper();

void kernel_main()
{
    clear_screen();

    print_string("Iniciando subsistema de interrupcoes...\n");

    // 1. Reprograma o chip controlador para não conflitar com a CPU
    pic_reprogram();
    print_string("-> PIC 8259 reprogramado com sucesso.\n");

    // 2. Inicializa a tabela estruturada na memória
    idt_init();
    print_string("-> IDT carregada no processador.\n\n");

    // REGISTRO DO TECLADO e RELOGIO NA IDT:
    // Adicionar o wrapper do relogio na linha 32 da tabela.
    // Adicionar o wrapper do teclado na linha 33 da tabela.
    // 0x08 é o seletor de código da GDT (CODE_SEG).
    // 0x8E define um portão de interrupção ativo com privilégio de Kernel.
    idt_set_gate(32, (unsigned int)timer_handler_wrapper, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)keyboard_handler_wrapper, 0x08, 0x8E);
    print_string("Teclado e Relogio mapeados na IDT!\n");

    pmm_init();
    print_string("-> Gerenciador de Memoria Fisica (PMM Bitmap) ativo.\n");

    heap_init();
    printf("-> Heap de Memoria (kmalloc) inicializado!\n\n");

    void *bloco1 = pmm_alloc_block();
    void *bloco2 = pmm_alloc_block();
    void *bloco3 = pmm_alloc_block();

    if (bloco1 != 0 && bloco2 != 0)
    {
        print_string("-> Sucesso! Bloco 1 alocado em: ");
        print_string("Endereco Valido.\n");
    }

    int teste_numero = 12345;

    printf("-> [PRINTF TEST] Numero: %d | Bloco1: 0x%x | Bloco2: 0x%x | Bloco3: 0x%x\n\n",
           teste_numero, (unsigned int)bloco1, (unsigned int)bloco2, (unsigned int)bloco3);

    // Devolve o bloco 1 para a RAM
    pmm_free_block(bloco1);
    print_string("-> Bloco 1 liberado de volta para a RAM com sucesso.\n\n");

    paging_init();
    print_string("-> Pagiancao Virtual de 32-bits ligada com sucesso!\n\n");

    print_string("==================================================\n\n");
    uart_init();
    print_string("-> UART/Serial (COM1) inicializada a 115200 bps.\n\n");

    uart_print("OBC_KERNEL: Subsistemas prontos em orbita");

    // Ativa as interrupções na CPU (Equivalente ao comando 'sti' em Assembly)
    __asm__ volatile("sti");
    char comando[256];
    int tamanho = 0;
    char *data_uart = (char *)kmalloc(16);

    while (1)
    {

        // Realiza a leitura dos dados do Serial
        char c = uart_read_nonblocking();

        if (c != 0)
        {
            // Exibe na tela o caractere e retorna uma mensagem de recebimento
            if (c == '\r' || c == '\n')
            {
                uart_print("\r\n[OBC] Pacote recebido com sucesso.\r\n");
            }
            else
            {
                data_uart[tamanho] = c;
                print_char(c);
                tamanho++;
            }
        }
        else
        {
            if (tamanho != NULL)
            {
                printf("\n[UART]: %s\n", data_uart);
                kfree(data_uart);
                tamanho = NULL;
            }

            read_command();
        }

        __asm__ volatile("hlt");
    }
}