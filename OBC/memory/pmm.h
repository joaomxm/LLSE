#ifndef PMM_H
#define PMM_H

#define BLOCK_SIZE 4096                             // Bloco de memoria de 4KB
#define RAM_MAX_SIZE 0x2000000                      // Simula um limite de 32MB de RAM
#define BITMAP_SIZE (RAM_MAX_SIZE / BLOCK_SIZE / 8) // Tamanho do bitmap em bytes (256 bytes)

void pmm_init();
void *pmm_alloc_block();
void pmm_free_block(void *p);
int pmm_test_bit(unsigned int block_index);

#endif