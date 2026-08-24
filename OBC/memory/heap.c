#include "heap.h"
#include "pmm.h"

// Ponteiro para o início da lista de blocos da Heap
static block_header_t *heap_start = NULL;

// Inicia o bloco vazio
void heap_init()
{
    heap_start = NULL;
}

// Expande a Heap pedindo uma nova página de 4KB ao PMM
static block_header_t *expand_heap(size_t min_size)
{
    // Aloca uma página física de 4KB (4096 bytes)
    void *page = pmm_alloc_block();

    if (page == NULL)
    {
        return NULL;
    }

    block_header_t *header = (block_header_t *)page;

    // O tamanho útil do bloco é 4096 bytes MENOS o tamanho do próprio header
    header->size = 4096 - sizeof(block_header_t);
    header->is_free = 1;
    header->next = NULL;

    return header;
}

void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    // Alinhamento de memória em 4 bytes para performance do x86
    size = (size + 3) & ~3;

    block_header_t *current = heap_start;
    block_header_t *last = NULL;

    // Procura um bloco livre existente que caiba o tamanho solicitado (First Fit)
    while (current != NULL)
    {
        if (current->is_free && current->size >= size)
        {
            // Opcional: Se sobrar espaço suficiente (ex: > 32 bytes), divide o bloco em dois (Splitting)
            if (current->size >= size + sizeof(block_header_t) + 16)
            {
                block_header_t *next_block = (block_header_t *)((char *)current + sizeof(block_header_t) + size);
                next_block->size = current->size - size - sizeof(block_header_t);
                next_block->is_free = 1;
                next_block->next = current->next;

                current->size = size;
                current->next = next_block;
            }

            current->is_free = 0;

            // Retorna o ponteiro após o cabeçalho (onde o usuário pode escrever)
            return (void *)(current + 1);
        }
        last = current;
        current = current->next;
    }

    // Se não achou nenhum bloco livre suficiente, pede mais 4KB ao PMM
    block_header_t *new_block = expand_heap(size);

    if (new_block == NULL)
    {
        return NULL;
    }
    else
    {
        last->next = new_block;
    }

    // Retorna a área de dados logo após o header
    return (void *)(new_block + 1);
}

// Libera o bloco que estava alocado
void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    block_header_t *header = ((block_header_t *)ptr) - 1;
    header->is_free = 1;
}