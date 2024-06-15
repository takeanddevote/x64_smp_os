#include "logger.h"
void print_hex(unsigned char *buf, int len)
{
    int i = 0; 
    if(len == 0)
        return;
    for(i = 0; i < len; i++){
        printf("%.2x", buf[i]);
    }
    printf(" %d.\n", len);
}

void print_hex_string(unsigned char *buf, int len, const char *str)
{
    int i = 0; 
    if(len == 0)
        return;
    printf("%s: ", str);
    for(i = 0; i < len; i++){
        printf("%.2x", buf[i]);
    }
    printf(" %d.\n", len);
}

int print_ts_string(struct timeval ts, const char *str)
{
    struct tm *tm_info;
    char buffer[64];

    // 获取当前时间戳
    gettimeofday(&ts, NULL);

    // 将秒部分转换为 `struct tm`，这里使用本地时间
    tm_info = localtime(&ts.tv_sec);

    // 格式化时间为可读格式
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    // 打印结果，并添加微秒部分
    printf("%s: %s.%06ld\n", str, buffer, ts.tv_usec);

    return 0;
}