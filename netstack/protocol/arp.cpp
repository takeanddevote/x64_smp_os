#include "arp.h"
#include "logger.h"
#include <sys/types.h>




void print_arp(arp_header *arp)
{
    struct in_addr in_addr;
    in_addr.s_addr = arp->sender_ip;
    printf("local ip %s.\n", inet_ntoa(in_addr));
    print_hex_string((unsigned char *)arp->sender_mac, ETH_ALEN, "local mac");

    in_addr.s_addr = arp->target_ip;
    printf("remote ip %s.\n", inet_ntoa(in_addr));
    print_hex_string((unsigned char *)arp->target_mac, ETH_ALEN, "remote mac");
}

void print_ether(const u_char *packet)
{
    struct ether_header *header = (struct ether_header *)packet;
    print_hex_string(header->ether_shost, ETH_ALEN, "src mac");
    print_hex_string(header->ether_dhost, ETH_ALEN, "dst mac");
    printf("protocol: %s.\n", ether_id_to_string(ntohs(header->ether_type)));
}

struct ether_header* create_ether_header(u_char* dst_mac, u_char* src_mac, ushort type) {
    struct ether_header* header = (struct ether_header* )calloc(sizeof(struct ether_header), 1);

    memcpy(header->ether_dhost, dst_mac, ETHER_ADDR_LEN);
    memcpy(header->ether_shost, src_mac, ETHER_ADDR_LEN);

    header->ether_type = htons(type);

    return header;
}

arp_header* create_arp_header(inet_info_t *inet) {
    arp_header* header = (arp_header* )calloc(sizeof(arp_header), 1);

    header->hardware_type = htons(0x0001);
    header->protocol_type = htons(ETHERTYPE_IP);
    header->hlen = 6;
    header->plen = 4;
    header->opcode = htons(ARPOP_REQUEST);

    memcpy(header->sender_mac, inet->local_mac, ETHER_ADDR_LEN);
    header->sender_ip = inet->local_ip;

    bzero(header->target_mac, ETHER_ADDR_LEN);
    header->target_ip = inet->remote_ip;

    return header;
}

void send_arp_req(inet_info_t *inet)
{
    int len = sizeof(struct ether_header) + sizeof(arp_header);
    char* packet = (char* )malloc(len);

    // 以太网协议头
    unsigned char broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct ether_header* ether = create_ether_header(broadcast_mac, inet->local_mac, ETHERTYPE_ARP);

    // arp request协议
    arp_header* arp = create_arp_header(inet);

    // 组合
    memcpy(packet, ether, sizeof(struct ether_header));
    memcpy(packet + sizeof(struct ether_header), arp, sizeof(arp_header));

    // 释放
    free(ether);
    free(arp);

    INFO_PRINT("start send ARP request package..\n");

    int sent = pcap_sendpacket(g_inet_info.handle, (const u_char *)(packet), len);
    if (sent < 0) {
        ERROR_PRINT("pcap_sendpacket failed: %s\n", pcap_geterr(g_inet_info.handle));
    } else {
        INFO_PRINT("[success] sent ARP request package..\n");
    }

    free(packet);
}

int deal_arp_reply(inet_info_t *inet, const u_char *packet)
{
    arp_header *arp = (arp_header *)(packet + sizeof(struct ether_header));

    // print_arp(arp);
    if(arp->sender_ip == inet->remote_ip && arp->target_ip == inet->local_ip    \
        /* && !memcpy(arp->sender_mac, inet->local_mac, ETHER_ADDR_LEN) */) {
        memcpy(inet->remote_mac, arp->sender_mac, ETHER_ADDR_LEN);
        // print_arp(arp);
        return 0;
    }
    return -1;
}