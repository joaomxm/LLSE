#include "util.h"

// Definicoes do tamanho da tela -> 25 colunas por 80 linhas = 2000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_ADDRESS 0xB8000 // Endereço inicial de video

int cursor_x;
int cursor_y;

// =============================================================================
// GERENCIADOR DE MEMÓRIA FÍSICA (PMM - BITMAP)
// =============================================================================

extern unsigned int _kernel_start;
extern unsigned int _kernel_end;

#define BLOCK_SIZE 4096                             // Bloco de memoria de 4KB
#define RAM_MAX_SIZE 0x2000000                      // Simula um limite de 32MB de RAM
#define BITMAP_SIZE (RAM_MAX_SIZE / BLOCK_SIZE / 8) // Tamanho do bitmap em bytes (256 bytes)

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

// =============================================================================
// GERENCIADOR DE MEMÓRIA VIRTUAL (PAGINAÇÃO)
// =============================================================================

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

    // 3. Coloca a nossa primeira Page Table na primeira posição do Page Directory
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

/*
=============================================================================
FUNÇÕES DE PORTA I/O (Conversa direta com os Chips)
=============================================================================

Função que envia um byte para a porta de de hardware (I/O)
outb %0, %1 -> intrucao de maquina para enviar o dado %0 para a porta %1
 : : "a"(data), "Nd"(port) ->coloca o conteudo de data no registrado AL e o numero da porta no regeistrador de hardware
*/

void outb(unsigned short port, unsigned char data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Função que escuta e retorna um byte que o chip de hardware deicou em uma porta especifica.
unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

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

// Cria a tabela IDT com espaço para todas as 256 interrupções possíveis
struct idt_entry_struct idt[256];
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
    idt_ptr.limit = (sizeof(struct idt_entry_struct) * 256) - 1; // Calcula o tamanho exato da tabela na memória (8 bytes por linha $\times$ 256 linhas $- 1$).
    idt_ptr.base = (unsigned int)&idt;                           // Pega o endereço da tabela idt começa na memória RAM.

    // Comando do compilador para carregar a tabela na CPU
    // lidt -> instrução Assembly nativa (Load Interrupt Descriptor Table)
    // Pega esse ponteiro descritor e injeta diretamente no coração do processador
    // CPU passa a usar a tabela em C para gerenciar o hardware
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

/*
=============================================================================
HANDLER DO TECLADO
=============================================================================
*/

const char keyboard_map[128] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8', /* 0-9 */
    '9',
    '0',
    '-',
    '=',
    '\b', /* Backspace */
    '\t', /* Tab */
    'q',
    'w',
    'e',
    'r', /* 16-19 */
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', /* Enter */
    0,    /* 29   - Control */
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';', /* 30-39 */
    '\'',
    '`',
    0, /* Left shift */
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.', /* 40-49 */
    '/',
    0, /* Right shift */
    '*',
    0,   /* Alt */
    ' ', /* Space bar */
    0,   /* Caps lock */
    0,   /* 59 - F1 key ... > */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0,
    0,
    0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /*Restante fica zerado */
};

extern void keyboard_handler_wrapper();

void keyboard_handler()
{
    // 1. Lê o "Scan Code" (código bruto da tecla) na porta 0x60
    unsigned char scancode = inb(0x60);

    // O chip do teclado envia um sinal quando a tecla é pressionada (bit 7 zerado)
    // e outro sinal quando ela é solta (bit 7 ativado, ou seja, valor maior que 0x80).
    // Ignora o momento de soltar a tecla por enquanto (valores maiores que 0x80).
    if (!(scancode & 0x80))
    {
        char c = keyboard_map[scancode];

        // Tradutor básico (Keymap):
        if (c != 0)
        {
            if (c == '\b')
            {
                // TODO: Pendente criar driver para remover caracteres
            }
            else
            {
                print_char(c);
            }
        }
    }

    // 2. AVISO CRÍTICO DE FIM DE INTERRUPÇÃO (EOI - End of Interrupt)
    // Enviar o byte 0x20 para a porta do PIC Master (0x20) para notificar o PIC que a CPU nao esta processando
    outb(0x20, 0x20);
}

/*
=============================================================================
HANDLER DO RELOGIO
=============================================================================
*/

extern void timer_handler_wrapper();
unsigned int timer_ticks = 0;

void timer_handler()
{
    timer_ticks++;

    // Endereço linear: (0 * 80 + 79) * 2 = 158 - para adicionar caractere para animacao
    char *video_memory = (char *)VIDEO_ADDRESS;

    // Adiciona animação simples usando os ticks (exibe '/' ou '-')
    if (timer_ticks & 0x20)
    {
        video_memory[158] = '/';
        video_memory[159] = 0x0E; // Texto amarelo
    }
    else
    {
        video_memory[158] = '-';
        video_memory[159] = 0x0E;
    }

    // Notifica o PIC Master que a interrupcao do relogio acabou
    outb(0x20, 0x20);
}

/*
=============================================================================
DRIVERS DE VIDEO - VGA
=============================================================================
*/

void clear_screen()
{

    char *video_memory = (char *)VIDEO_ADDRESS; // Cria um ponteiro para o endereco de memora
    char attribute_byte = 0x0F;                 // Cor preta com fundo branco

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) // percorre todos os enderecos aplicando o espcao em branco e fundo preto
    {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = attribute_byte;
    }

    // Posicao inicial para o cursor
    cursor_x = 0;
    cursor_y = 0;
}

void print_char(char c)
{
    char *video_memory = (char *)VIDEO_ADDRESS;
    char attribute_byte = 0x0F;

    if (c == '\n') // Verifica se é quebra de linha (se for incrementa os cursores)
    {
        cursor_x = 0;
        cursor_y++;
        return;
    }

    if (c == '\t') // Verifica se é tabulação (se for incrementa o cursor x)
    {
        cursor_x += 4;
        return;
    }

    // Calculo da posicao do cursor para print do caractere
    // Ex: (0 * 80 + 0) *2  = 2 (Primeira linha, primeira coluna)
    int memory_offset = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;

    video_memory[memory_offset] = c;
    video_memory[memory_offset + 1] = attribute_byte;

    cursor_x++;

    // Se o cursor_x ultrapassar 80 significa que esta fora do offset, precisa ir para proxima linha e primeira coluna
    if (cursor_x >= SCREEN_WIDTH)
    {
        cursor_x = 0;
        cursor_y++;
    }
}

void print_string(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++) // Verifica cada caractere até encontrar o final da string que é '\0'
    {
        print_char(str[i]);
    }
}

/*
=============================================================================
FUNÇÃO PRINCIPAL - SISTEMA
=============================================================================
*/

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

    // Ativa as interrupções na CPU (Equivalente ao comando 'sti' em Assembly)
    __asm__ volatile("sti");
    print_string("Digite algo:\n> ");

    while (1)
    {
        __asm__ volatile("hlt");
    }
}