#ifndef __UDP_H__
#define __UDP_H__
#include "common.h"
#include "protocol_cache.h"


// 伪包头（tcp、udp用）
typedef struct {
    uint32_t src_addr;      // 源IP地址
    uint32_t dest_addr;     // 目的IP地址
    uint8_t  reserved;      // 保留字节，置0
    uint8_t  protocol;      // 协议号，常为6表示TCP协议
    uint16_t length;    // TCP头部长度
} __attribute__((packed)) pseudo_header;

// UDP协议头的结构体
typedef struct udp_header {
    uint16_t source_port;   // 源端口号
    uint16_t dest_port;     // 目标端口号
    uint16_t length;        // 数据报长度
    uint16_t checksum;      // 校验和
} __attribute__((packed)) udp_header;

void *udp_send(inet_info_t *inet, void *data, size_t len);

#endif /* __UDP_H__ */
