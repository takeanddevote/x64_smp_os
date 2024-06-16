#include "ip.h"
#include "icmp.h"
#include "protocol_cache.h"
#include "util.h"
#include "udp.h"
#include "tcp.h"
#include <cstddef>


ip_header* create_ip_icmp_protocol_header(inet_info_t *inet, uint8_t protocol) 
{
    ip_header *header = (ip_header *)calloc(sizeof(ip_header), 1);

    header->version_ihl = 0x45;
    header->tos = 0;
    header->total_length = htons(sizeof(ip_header) + sizeof(icmp_header));
    header->identification = htons(1);
    header->flags_fragment = htons(0x4000);
    header->ttl = 64;
    header->protocol = protocol;
    header->checksum = 0;
    header->src_ip = inet->local_ip;
    header->dst_ip = inet->remote_ip;

    header->checksum = ip_checksum((uint16_t *)header, sizeof(ip_header));

    return header;
}

ip_header* create_ip_header(uint8_t tos, uint16_t len, uint16_t identification, uint16_t flags, uint8_t ttl,
                            uint8_t protocol, uint32_t src_ip, uint32_t dst_ip) 
{
    ip_header *header = (ip_header *)calloc(sizeof(ip_header), 1);

    header->version_ihl = 0x45;
    header->tos = tos;
    header->total_length = len;
    header->identification = htons(identification);
    header->flags_fragment = htons(flags);
    header->ttl = ttl;
    header->protocol = protocol;
    header->checksum = 0;
    header->src_ip = src_ip;
    header->dst_ip = dst_ip;

    header->checksum = ip_checksum((uint16_t *)header, sizeof(ip_header));

    return header;
}


int distribute_ip_reply(const u_char *packet) /* 分发ip协议回复包 */
{
    int ret = 0;
    size_t tcplen;
    ip_header *ip = (ip_header *)(packet + sizeof(struct ether_header));

    // std::cout << "src ip " << nip_to_ascall(ip->src_ip) << " dst ip " << nip_to_ascall(ip->dst_ip) << std::endl;
    if(ip->src_ip != g_inet_info.remote_ip || ip->dst_ip != g_inet_info.local_ip) {
        return 0;
    }

    switch(ip->protocol) {
        case IP_LOAD_ICMP:
            ret = distribute_icmp_reply((icmp_header *)(packet + sizeof(struct ether_header) + sizeof(ip_header)));
            break;

        case IP_LOAD_TCP:
            tcplen = ntohs(ip->total_length) - sizeof(ip_header);
            distribute_tcp_recv((tcp_header *)(packet + sizeof(struct ether_header) + sizeof(ip_header)), tcplen);
            break;

        case IP_LOAD_UDP:
            distribute_udp_recv((udp_header *)(packet + sizeof(struct ether_header) + sizeof(ip_header)));
            break;

        default:
            break;
    }
    return ret;
}