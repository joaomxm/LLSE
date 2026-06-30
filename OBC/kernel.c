
// Definicoes do tamanho da tela -> 25 colunas por 80 linhas = 2000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_ADDRESS 0xB8000 // Endereço inicial de video

int cursor_x;
int cursor_y;

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

void kernel_main()
{
    clear_screen();

    print_char('C');
    print_char(' ');
    print_string("Teste String");

    while (1)
    {
        // Loop infinito
    }
}