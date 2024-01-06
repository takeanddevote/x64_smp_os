
void delay_ms(int ms)
{
    for(int i = 0; i < ms; ++i) {
        for(int j = 0; j < 5000; ++j) {
            j = j;
        }
    }
}