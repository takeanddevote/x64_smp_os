#ifndef __ICMP_H__
#define __ICMP_H__
#include "common.h"
#include "protocol_cache.h"
// ICMP协议头后面可以带data
typedef struct {
    uint8_t  type;           // icmp类型    (8-9)-回显请求/应答，测试连通性 (13-14)-时间戳请求/应答，测试网络延迟
    uint8_t  code;           // 代码
    uint16_t checksum;       // 校验和
    uint16_t identifier;     // 标识符
    uint16_t sequence_num;   // 序列号
} __attribute__((packed)) icmp_header;

#define ICMP_TYPE_REQ_ECHO      8
#define ICMP_TYPE_REPLY_ECHO    0
#define ICMP_TYPE_REQ_TS        13
#define ICMP_TYPE_REPLY_TS      14


icmp_header* create_icmp_protocol_header(uint8_t type);
void send_icmp_echo_request_packet(inet_info_t *inet);  /* 发送回显请求，测试连通性 */
int distribute_icmp_reply(icmp_header *icmp);

#endif /* __ICMP_H__ */
