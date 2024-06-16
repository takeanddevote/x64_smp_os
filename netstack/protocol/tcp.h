#ifndef __TCP_H__
#define __TCP_H__
#include "common.h"
#include "protocol_cache.h"

#define TCP_URG     0x20
#define TCP_ACK     0x10
#define TCP_PSH     0x08
#define TCP_RST     0x04
#define TCP_SYN     0x02
#define TCP_FIN     0x01

#define TCP_SYN_ACK     TCP_SYN | TCP_ACK
#define TCP_PSH_ACK     TCP_PSH | TCP_ACK


typedef struct tcp_header {
    uint16_t source_port;     // 源端口
    uint16_t dest_port;       // 目的端口
    uint32_t sequence_num;    // 序列号。客户端各自生成自己的序列号
    uint32_t ack_num;         // 确认号。对方的序列号加上一个值。
    uint8_t data_offset;      // 数据偏移，指示TCP头部的长度
    uint8_t flags;            // 标识TCP头部的不同标志 0 0 URG ACK PSH RST SYN FIN
    uint16_t window_size;     // 窗口大小
    uint16_t checksum;        // 校验和
    uint16_t urgent_pointer;  // 紧急指针
} __attribute__((packed)) tcp_header;

void tcp_init_seq(inet_info_t *inet);
void tcp_send_control(inet_info_t *inet, uint8_t flag);
void tcp_send(inet_info_t *inet, char *data, size_t d_len);
int distribute_tcp_recv(struct tcp_header *tcp);

#endif /* __TCP_H__ */
