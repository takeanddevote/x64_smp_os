#include "logger.h"

void print_hex(unsigned char *buf, int len)
{
    int i = 0; 
    if(len == 0)
        return;
    for(i = 0; i < len; i++){
        p_printf("%.2x", buf[i]);
    }
    p_printf(" %d.\n", len);
}


void print_nstring_label(unsigned char *buf, int len, char *label)
{
    char str[100] = {0};

    if(len == 0)
        return;
    memcpy(str, buf, len);
    p_printf("[%s]:%s.\n", label, str);
}