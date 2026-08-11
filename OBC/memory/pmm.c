#include "pmm.h"

extern unsigned int _kernel_start;
extern unsigned int _kernel_end;

// // Array do bitmap (inicia com tudo 1 = Ocupado por seguranca)
volatile unsigned char pmm_bitmap[BITMAP_SIZE];

// Funcao para setar um bit (1 = Ocupado)
void pmm_set_bit(unsigned int block_index)
{
    unsigned int byte = block_index / 8;
    unsigned int bit = block_index % 8;
    pmm_bitmap[byte] |= (1 << bit);
}

// Funcao para limpar um bit (0 = livre)
void pmm_clear_bit(unsigned int block_index)
{
    unsigned int byte = block_index / 8;
    unsigned int bit = block_index % 8;
    pmm_bitmap[byte] &= ~(1 << bit);
}

// Funcao para ler o estado de um bit
int pmm_test_bit(unsigned int block_index)
{
    unsigned int byte = block_index / 8;
    unsigned int bit = block_index % 8;
    return (pmm_bitmap[byte] & (1 << bit)) != 0;
}

// Inicializa o gerenciador marcando o que é livre e o que é Kernel
void pmm_init()
{
    // 1. Inicialmente, assume que toda a memória está ocupada (setada em 1)
    for (int i = 0; i < BITMAP_SIZE; i++)
    {
        pmm_bitmap[i] = 0xFF;
    }

    // 2. Libera a memória RAM padrão (de 1MB até 32MB)
    // Os primeiros 1MB (0x000000 a 0x100000) são reservados para hardware/VGA da BIOS, deixa bloqueados.
    unsigned int start_block = 0x100000 / BLOCK_SIZE; // Bloco correspondente a 1MB
    unsigned int end_block = RAM_MAX_SIZE / BLOCK_SIZE;

    for (unsigned int i = start_block; i < end_block; i++)
    {
        pmm_clear_bit(i); // Marca como LIVRE (0)
    }

    // 3. Bloqueia a área exata onde o Kernel está rodando na RAM para proteção
    unsigned int kernel_start_block = ((unsigned int)&_kernel_start) / BLOCK_SIZE;
    unsigned int kernel_end_block = (((unsigned int)&_kernel_end) / BLOCK_SIZE) + 1;

    for (unsigned int i = kernel_start_block; i < kernel_end_block; i++)
    {
        pmm_set_bit(i); // Marca como OCUPADO (1) - Proteção do Kernel!
    }
}

// Aloca o primeiro bloco de 4KB livre que encontrar - malloc fisico
void *pmm_alloc_block()
{
    unsigned int total_blocks = RAM_MAX_SIZE / BLOCK_SIZE;

    unsigned int start_search_block = 0x100000 / BLOCK_SIZE;

    for (unsigned int i = start_search_block; i < total_blocks; i++)
    {
        if (pmm_test_bit(i) == 0)
        {                   // Encontrou um bloco livre!
            pmm_set_bit(i); // Marca como ocupado agora
            unsigned int addr = i * BLOCK_SIZE;
            return (void *)addr; // Retorna o endereço real de memória RAM de 4KB
        }
    }
    return 0; // Out of Memory (RAM cheia!)
}

// Libera um bloco de volta para o sistema - free fisico
void pmm_free_block(void *p)
{
    unsigned int addr = (unsigned int)p;
    unsigned int block_index = addr / BLOCK_SIZE;
    pmm_clear_bit(block_index); // Marca como livre no Bitmap
}
