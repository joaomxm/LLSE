#ifndef IDT_H
#define IDT_H

// Define uma entrada (Gate) da IDT de exatamente 8 bytes
struct idt_entry_struct
{
    unsigned short offset_1; // Bits 0-15 do endereço da função
    unsigned short selector; // Seletor de segmento de código da GDT (CODE_SEG)
    unsigned char zero;      // Este byte sempre deve ser 0
    unsigned char flags;     // Atributos de tipo e privilégio
    unsigned short offset_2; // Bits 16-31 do endereço da função
} __attribute__((packed));   // 'packed' impede o compilador de otimizar o tamanho

// Define o ponteiro descritor que a CPU usa para ler a IDT (semelhante à GDT)
struct idt_ptr_struct
{
    unsigned short limit;
    unsigned int base;
} __attribute((packed));

typedef struct idt_entry_struct idt_entry_struct_t;

void pic_reprogram();
void idt_init();
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags);

#endif