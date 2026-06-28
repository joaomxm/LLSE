
// Definicoes do tamanho da tela -> 25 colunas por 80 linhas = 2000
#define SCREEN_WIDTH 25
#define SCREEN_HEIGHT 80
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

void kernel_main()
{
    clear_screen();

    char *video_memory = (char *)0xB8000;

    video_memory[4] = 'C';
    video_memory[5] = 0x4;

    while (1)
    {
        // Loop infinito
    }
}