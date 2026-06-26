void kernel_main()
{

    char *video_memory = (char *)0xB8000;

    video_memory[4] = 'C';
    video_memory[5] = 0x4;

    while (1)
    {
        // Loop infinito
    }
}