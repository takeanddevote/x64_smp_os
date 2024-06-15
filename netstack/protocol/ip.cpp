#include "ip.h"
#include "icmp.h"
#include "protocol_cache.h"
#include "util.h"


ip_header* create_ip_protocol_header(inet_info_t *inet, uint8_t protocol) 
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


int distribute_ip_reply(const u_char *packet) /* 分发ip协议回复包 */
{
    int ret = 0;
    ip_header *ip = (ip_header *)(packet + sizeof(struct ether_header));
    switch(ip->protocol) {
        case IP_LOAD_ICMP:
            ret = distribute_icmp_reply((icmp_header *)(packet + sizeof(struct ether_header) + sizeof(ip_header)));
            break;

        case IP_LOAD_TCP:
            break;

        case IP_LOAD_UDP:
            break;

        default:
            break;
    }
    return ret;
}