__attribute__((section(".entry"), used)) void _main()
{
    volatile int truc = 0;
    while (1)
    {
        truc++;
        asm volatile("int 128");
        for (int i = 0; i < 1000000; i++)
        {
            asm volatile("nop");
        }
    }
    return truc;

}
