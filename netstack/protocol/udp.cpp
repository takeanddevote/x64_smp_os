#include "udp.h"
#include "protocol_cache.h"
#include "util.h"
#include "arp.h"
#include "ip.h"
#include <cstddef>

uint16_t udp_checksum(pseudo_header *psh, udp_header *udph, uint16_t *data, int data_len) {
    int total_len = sizeof(pseudo_header) + sizeof(struct udp_header) + data_len;
    uint16_t *buffer = (uint16_t*)malloc(total_len);
    uint16_t *buffer_ptr = buffer;

    memcpy(buffer_ptr, psh, sizeof(pseudo_header));
    buffer_ptr += sizeof(pseudo_header) / 2;

    memcpy(buffer_ptr, udph, sizeof(struct udp_header));
    buffer_ptr += sizeof(struct udp_header) / 2;

    memcpy(buffer_ptr, data, data_len);

    uint16_t checksum = ip_checksum(buffer, total_len);

    free(buffer);

    return checksum;
}




udp_header* create_udp_header(inet_info_t *inet, void *data, size_t d_len) 
{
    // 计算整个udp包的大小 = udp包头 + 数据
    uint16_t len = sizeof(struct udp_header) + d_len;

    // udp头
    udp_header *header = (udp_header *)calloc(len, 1);
    header->source_port = htons(inet->local_port);
    header->dest_port = htons(inet->remote_port);
    header->length = htons(len);    /* udp承载的数据长度 */
    header->checksum = 0;

    // 将数据copy到udp包头后面
    if (0 != d_len) {
        memcpy((u_char*)header + sizeof(struct udp_header), data, d_len);
    }

    // udp伪包头，用于计算校验和
    pseudo_header *ps_header = (pseudo_header *)calloc(sizeof(pseudo_header), 1);
    ps_header->src_addr = inet->local_ip;
    ps_header->dest_addr = inet->remote_ip;
    ps_header->reserved = 0;
    ps_header->protocol = IPPROTO_UDP;
    ps_header->length = htons(len);

    // 计算checksum
    u_char *tmp = (u_char *)calloc(sizeof(pseudo_header) + len, 1);
    memcpy(tmp, ps_header, sizeof(pseudo_header));
    memcpy(tmp + sizeof(pseudo_header), header, len);

    header->checksum = udp_checksum(ps_header, header, (uint16_t *)data, d_len);

    // 释放
    free(tmp);
    free(ps_header);

    return header;
}


void *udp_send(inet_info_t *inet, void *data, size_t len) 
{
    // 等监控线程启动起来
    sleep(1);

    // 以太网包头
    struct ether_header* ethernet_protocol_header = create_ether_header(inet->remote_mac, inet->local_mac, ETHERTYPE_IP);

    // 创建udp数据包
    struct udp_header* udp_header = create_udp_header(&g_inet_info, data, len);

    // IP包头
    uint16_t ip_header_len = sizeof(ip_header) + sizeof(struct udp_header) + len;
    ip_header* ipheader = create_ip_header(0, htons(ip_header_len), 0x3fa6, 0x4000, 64,     \
                IP_LOAD_UDP, inet->local_ip, inet->remote_ip);

    // 组合
    int packet_len = sizeof(struct ether_header) + sizeof(ip_header) + sizeof(struct udp_header) + len;
    u_char *packet = ( u_char *)calloc(packet_len, 1);
 
    memcpy(packet, ethernet_protocol_header, sizeof(struct ether_header));
    memcpy(packet + sizeof(struct ether_header), ipheader, sizeof(ip_header));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header), udp_header, sizeof(struct udp_header) + len);

    int sent = pcap_sendpacket(inet->handle, packet, packet_len);
    if (sent < 0) {
        ERROR_PRINT("pcap_sendpacket failed: %s\n", pcap_geterr(inet->handle));
        exit(-1);
    } else {
        INFO_PRINT("[success] sent UDP package..\n");
    }

    return NULL;
}