

int x64_kernel_main()
{
    console_init();
    char str[] = "enter x64_kernel_main success...";
    console_write(str, sizeof(str));
    while(1);
    
    return 0;
}