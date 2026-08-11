#include "paging.h"

// Alinha as tabelas em fronteiras de 4KB (Obrigatório para o processador!)
unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int first_page_table[1024] __attribute__((aligned(4096)));

void paging_init()
{
    // 1. Preenche todo o Page Directory com entradas vazias (Não Presentes)
    // O valor 0x02 significa: Não Presente, mas com permissão de Escrita/Leitura
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0x00000002;
    }

    // 2. Mapeia o primeiro megabyte de memória (Identity Mapping)
    // Isso vai cobrir do endereço 0x00000000 até 0x003FFFFF (4MB de RAM)
    // Garante que o Kernel, a pilha e a VGA continuem no mesmo lugar virtual
    for (unsigned int i = 0; i < 1024; i++)
    {
        // Assegura o endereço físico real associado às flags (0x03 = Presente + Escrita)
        first_page_table[i] = (i * 4096) | 3;
    }

    // 3. Coloca a primeira Page Table na primeira posição do Page Directory
    // O endereço da tabela precisa das flags de controle também (0x03)
    page_directory[0] = ((unsigned int)first_page_table) | 3;

    // 4. ATIVAÇÃO NO PROCESSADOR (Mágica do Assembly)
    // Carrega o endereço do Page Directory no registrador de controle CR3
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));

    // Lê o registrador CR0, ativa o bit mais alto (Bit 31 - PG / Paging) e grava de volta
    unsigned int cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Bit 31 ativa a paginação
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
