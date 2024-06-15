#ifndef __ARP_H__
#define __ARP_H__
#include "common.h"
#include "logger.h"
#include "util.h"
#include <stdint.h>
#include <stdint.h>
#include "protocol_cache.h"


#define ETH_ALEN 6   // 以太网硬件地址长度（6 字节）
#define IPV4_ALEN 4  // IPv4 地址长度（4 字节）

// ARP 数据包结构体定义
typedef struct {
    short   hardware_type;      // 硬件类型，以太网的硬件类型为1（大端）
    short   protocol_type;      // 协议类型，IPv4的协议类型为0x0800（大端）
    char    hlen;               // 硬件地址长度，即MAC地址的长度，为6个字节
    char    plen;               // 协议地址长度，比如IPv4，为4个字节
    short   opcode;             // 操作码，如果是request包，填充1，如果是reply包，填充2（大端）
    char    sender_mac[ETHER_ADDR_LEN];
    uint32_t   sender_ip;
    char    target_mac[ETHER_ADDR_LEN];
    uint32_t   target_ip;
} __attribute__((packed)) arp_header;

void send_arp_req(inet_info_t *inet);
int deal_arp_reply(inet_info_t *inet, const u_char *packet);
void print_ether(const u_char *packet);

#endif /* __ARP_H__ */
