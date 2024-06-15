#include "icmp.h"
#include "arp.h"
#include "logger.h"
#include "protocol_cache.h"
#include "ip.h"
#include "thread.h"
#include "util.h"

icmp_header* create_icmp_protocol_header(uint8_t type) {
    icmp_header *header = (icmp_header *)calloc(sizeof(icmp_header), 1);

    header->type = type;
    header->code = 0;
    header->checksum = 0;
    header->identifier = htons(1);
    header->sequence_num = htons(1);

    header->checksum = ip_checksum((uint16_t *)header, sizeof(icmp_header));

    return header;
}


void send_icmp_echo_request_packet(inet_info_t *inet) {
    int len = sizeof(struct ether_header) + sizeof(arp_header) + sizeof(icmp_header);
    char *packet = (char *)calloc(len, 1);

    // 以太网协议
    struct ether_header* ether = create_ether_header(inet->remote_mac, inet->local_mac, ETHERTYPE_IP);

    // 承载icmp协议的ip协议
    ip_header* ip = create_ip_icmp_protocol_header(&g_inet_info, IP_LOAD_ICMP);

    // icmp协议，类型为8，即回显请求，测试连通性
    icmp_header* icmp = create_icmp_protocol_header(ICMP_TYPE_REQ_ECHO);

    // 组合
    memcpy(packet, ether, sizeof(struct ether_header));
    memcpy(packet + sizeof(struct ether_header), ip, sizeof(ip_header));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header), icmp, sizeof(icmp_header));

    // 释放
    free(icmp);
    free(ip);
    free(ether);

    printf("start send ICMP echo request package..\n");

    int sent = pcap_sendpacket(g_inet_info.handle, (const u_char *)packet, len);
    if (sent < 0) {
        ERROR_PRINT("pcap_sendpacket failed: %s\n", pcap_geterr(g_inet_info.handle));
    } else {
        INFO_PRINT("[success] sent ICMP echo request package..\n");
    }

    free(packet);
}

int distribute_icmp_reply(icmp_header *icmp)
{
    int ret = 0;
    // printf("type %d.\n", icmp->type);
    switch(icmp->type) {
        case ICMP_TYPE_REPLY_ECHO:  /* icmp回显回复，唤醒请求线程，即ping通了 */
            g_inet_info.ping_success = true;
            nst_post_by_name("icmp_req_handle");
            break;
        case ICMP_TYPE_REPLY_TS:
            break;
        default:
            break;
    }

    return ret;
}