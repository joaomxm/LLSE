#include "idt.h"
#include "io.h"
/*
=============================================================================
ESTRUTURAS DA IDT (Tabela de Interrupções)
=============================================================================

IDT - Interrupt Descriptor Table, tabela fisica na memoria RAM que contem 256 entradas (Gates/Portoes),
Cada entrada é uma estrutura de 8bytes que aponta para o endereco de uma funcao na memoria,
Quando acontece uma Interrupção (erro divisao por zero, tecla pressionada...),
a CPU usa o numero da interrupcao como indice nessa tabela para salta direto para a funcao correspondente.

- Offset (Bits 0-15 e 16-31): O endereço de memória da função em C que vai tratar a interrupção. Ele fica dividido em duas partes na estrutura por motivos históricos de compatibilidade.
- Selector (Bits 16-31): O segmento de código da GDT (o CODE_SEG do Stage 2).
- Flags (Bits 40-47): Define se o portão está ativo, o nível de privilégio (Ring 0) e o tipo de portão (geralmente 0x8E para portões de interrupção de 32 bits).
*/

// Cria a tabela IDT com espaço para todas as 256 interrupções possíveis
idt_entry_struct_t idt[256]; // Outra forma da struct
struct idt_ptr_struct idt_ptr;

/*
=============================================================================
REPROGRAMAÇÃO DO CHIP PIC 8259
=============================================================================

PIC 8259 - gerencia as interrupcoes externas de hardware.
IRQs -> Interrupts Requests
Função para reprogramar os vetores de interrupcao deslocando as IRQs de hardware para comecar a partir do vetor 0x20 (32) em diante,
No modo protegido a Inter reserva 0 a 31 exclusivamente para Excecoes do Processador
*/
void pic_reprogram()
{
    // ICW1 - Inicialização do PIC Master e Slave
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2 - Mapeia o vetor de destino das IRQs
    outb(0x21, 0x20); // Master começa em 0x20 (Interrupção 32 decimal)
    outb(0xA1, 0x28); // Slave começa em 0x28 (Interrupção 40 decimal)

    // ICW3 - Configura a conexão em cascata entre os dois chips
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4 - Define o modo de operação (Modo 8086)
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Ativa apenas o Relógio (IRQ0) e o Teclado (IRQ1), mascarando/desativando o resto
    // Bit 0 = IRQ0, Bit 1 = IRQ1. O valor 0 ativa e 1 desativa.
    // 0xFC em binário é 11111100 (Ativa bits 0 e 1, desativa do 2 ao 7)
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF); // Desativa todas as IRQs do Slave
}

/*
=============================================================================
CONFIGURAÇÃO DA IDT
=============================================================================

Função idt_set_gate
- num: Qual a linha da tabela (ex: linha 33 para o teclado).
- base: O endereço de memória real da função que vai tratar o teclado.
- sel: O segmento de código da GDT (CODE_SEG).
- flags: Permissões de segurança (geralmente 0x8E, que significa "Portão ativo de 32 bits em nível de Kernel").
*/

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags)
{
    idt[num].offset_1 = base & 0xFFFF; // Pega apenas os 16 bits finais do endereço e guarda na primeira gaveta.
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].offset_2 = (base >> 16) & 0xFFFF; // Empurra o endereço 16 bits para a direita para pegar a metade inicial dele e guarda na segunda gaveta.
}

// Registrador interno da CPU precisa receber o ponteiro da IDT via comando Assembly lidt
void idt_init()
{
    idt_ptr.limit = (sizeof(idt_entry_struct_t) * 256) - 1; // Calcula o tamanho exato da tabela na memória (8 bytes por linha $\times$ 256 linhas $- 1$).
    idt_ptr.base = (unsigned int)&idt;                      // Pega o endereço da tabela idt começa na memória RAM.

    // Comando do compilador para carregar a tabela na CPU
    // lidt -> instrução Assembly nativa (Load Interrupt Descriptor Table)
    // Pega esse ponteiro descritor e injeta diretamente no coração do processador
    // CPU passa a usar a tabela em C para gerenciar o hardware
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}