#ifndef __IP_H__
#define __IP_H__
#include "common.h"
#include "protocol_cache.h"

typedef struct {
    uint8_t  version_ihl;     // 版本和头部长度
    uint8_t  tos;             // 服务类型
    uint16_t total_length;    // 总长度
    uint16_t identification;  // 标识符
    uint16_t flags_fragment;  // 标志和片偏移
    uint8_t  ttl;             // 生存时间
    uint8_t  protocol;        // 承载协议类型 1-icmp 6-tcp 17-udp
    uint16_t checksum;        // 校验和
    uint32_t src_ip;          // 源IP地址
    uint32_t dst_ip;          // 目的IP地址
} __attribute__((packed)) ip_header;

#define IP_LOAD_ICMP    1
#define IP_LOAD_TCP     6
#define IP_LOAD_UDP     17

ip_header *create_ip_icmp_protocol_header(inet_info_t *inet, uint8_t protocol);
int distribute_ip_reply(const u_char *packet);
ip_header* create_ip_header(uint8_t tos, uint16_t len, uint16_t identification, uint16_t flags, uint8_t ttl,
                            uint8_t protocol, uint32_t src_ip, uint32_t dst_ip);

#endif /* __IP_H__ */
