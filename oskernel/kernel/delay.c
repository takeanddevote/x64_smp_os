
void delay_ms(int ms)
{
    int flag = 0xff;
    for(int i = 0; i < ms; ++i) {
        for(int j = 0; j < 81500; ++j) {
            j = j;
        }
    }
    flag = 0x4f;
}