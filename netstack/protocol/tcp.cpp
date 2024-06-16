#include "tcp.h"
#include "logger.h"
#include "thread.h"
#include "udp.h"
#include "arp.h"
#include "ip.h"
#include "protocol_cache.h"
#include "util.h"
#include <cstdlib>

tcp_header* create_tcp_header(inet_info_t *inet, uint8_t flag, uint32_t seq_num, uint32_t ack_num, u_char* options, size_t options_len, char* s, size_t s_len) {
    pseudo_header *pseudo = (pseudo_header *)calloc(sizeof(pseudo_header), 1);
    pseudo->src_addr = inet->local_ip;
    pseudo->dest_addr = inet->remote_ip;
    pseudo->reserved = 0;
    pseudo->protocol = 6;
    pseudo->length = htons(sizeof(tcp_header) + options_len + s_len);

    tcp_header *tcp = (tcp_header *)calloc(sizeof(tcp_header), 1);
    tcp->source_port = htons(inet->local_port);
    tcp->dest_port = htons(inet->remote_port);
    tcp->sequence_num = htonl(seq_num);
    tcp->ack_num = htonl(ack_num);
    tcp->data_offset = (sizeof(tcp_header) + options_len) / 4 << 4;            // tcp header + tcp options的长度
    tcp->flags = flag;
    tcp->window_size = htons(0xfaf0);
    tcp->checksum = 0;
    tcp->urgent_pointer = 0;

    ushort checksum = tcp_checksum((const uint8_t *)pseudo, sizeof(pseudo_header), (const uint8_t *)tcp, sizeof(tcp_header),
                                   (const uint8_t *)options, options_len, (const uint8_t *)s, s_len);

    tcp->checksum = htons(checksum);

    return tcp;
}

ip_header* create_ip_protocol_header(inet_info_t *inet, uint16_t len, uint8_t protocol) {
    ip_header *header = (ip_header *)calloc(sizeof(ip_header), 1);

    header->version_ihl = 0x45;
    header->tos = 0;
    header->total_length = len;
    header->identification = htons(0xd0f1);
    header->flags_fragment = htons(0x4000);
    header->ttl = 64;
    header->protocol = protocol;
    header->checksum = 0;
    header->src_ip = inet->local_ip;
    header->dst_ip = inet->remote_ip;

    header->checksum = ip_checksum((uint16_t *)header, sizeof(ip_header));

    return header;
}

void tcp_init_seq(inet_info_t *inet)
{
    inet->seq_num = rand();
    inet->ack_num = 0;
    inet->tcp_status = TCP_CLOSED;
}


void tcp_send_control(inet_info_t *inet, uint8_t flag) 
{
    u_char tcp_options[] = {
            0x02, 0x04, 0x05, 0xb4, 0x04, 0x02, 0x08, 0x0a, 0xaf, 0xef,
            0x15, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07
    };
    uint32_t seq_num = inet->seq_num;
    uint32_t ack_num = inet->ack_num;

    tcp_header* tcp = create_tcp_header(&g_inet_info, flag, seq_num, ack_num, tcp_options, sizeof(tcp_options), NULL, 0);

    struct ether_header* ethernet = create_ether_header(g_inet_info.remote_mac, g_inet_info.local_mac, ETHERTYPE_IP);
    ip_header* ip = create_ip_protocol_header(&g_inet_info, htons(sizeof(ip_header) + sizeof(tcp_header) + sizeof(tcp_options)), IP_LOAD_TCP);

    int packet_len = sizeof(struct ether_header) + sizeof(ip_header) + sizeof(tcp_header) + sizeof(tcp_options);
    u_char *packet = (u_char *)calloc(packet_len, 1);

    memcpy(packet, ethernet, sizeof(struct ether_header));
    memcpy(packet + sizeof(struct ether_header), ip, sizeof(ip_header));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header), tcp, sizeof(tcp_header));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header) + sizeof(tcp_header), tcp_options, sizeof(tcp_options));

    free(ethernet);
    free(ip);
    free(tcp);

    int sent = pcap_sendpacket(g_inet_info.handle, packet, packet_len);
    if (sent < 0) {
        ERROR_PRINT("pcap_sendpacket failed: %s\n", pcap_geterr(g_inet_info.handle));
        exit(-1);
    } else {
        INFO_PRINT("[success] sent TCP SYN package..\n");
    }

    free(packet);
}

void tcp_send(inet_info_t *inet, char *data, size_t d_len) 
{
//    u_char tcp_options[] = {0x01, 0x01, 0x08, 0x0a, 0xc4, 0x5e, 0x5b, 0xa6, 0xee, 0x28, 0x99, 0x81};
    uint32_t seq_num = inet->seq_num;
    uint32_t ack_num = inet->ack_num;
    uint8_t flag = TCP_PSH_ACK;

    tcp_header* tcp = create_tcp_header(&g_inet_info, flag, seq_num, ack_num, NULL, 0, data, d_len);

    struct ether_header* ethernet = create_ether_header(g_inet_info.remote_mac, g_inet_info.local_mac, ETHERTYPE_IP);
    ip_header* ip = create_ip_protocol_header(&g_inet_info, htons(sizeof(ip_header) + sizeof(tcp_header) +  d_len), IP_LOAD_TCP);

    int packet_len = sizeof(struct ether_header) + sizeof(ip_header) + sizeof(tcp_header)  + d_len;
    u_char *packet = (u_char *)calloc(packet_len, 1);

    memcpy(packet, ethernet, sizeof(struct ether_header));
    memcpy(packet + sizeof(struct ether_header), ip, sizeof(ip_header));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header), tcp, sizeof(tcp_header));
//    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header) + sizeof(tcp_header), tcp_options, sizeof(tcp_options));
    memcpy(packet + sizeof(struct ether_header) + sizeof(ip_header) + sizeof(tcp_header) , data, d_len);

    free(ethernet);
    free(ip);
    free(tcp);

    int sent = pcap_sendpacket(g_inet_info.handle, packet, packet_len);
    if (sent < 0) {
        ERROR_PRINT("pcap_sendpacket failed: %s\n", pcap_geterr(g_inet_info.handle));
        exit(-1);
    } else {
        INFO_PRINT("[success] sent TCP SYN package..\n");
    }

    free(packet);
}

int distribute_tcp_ack(struct tcp_header *tcp)
{
    if(g_inet_info.tcp_status == TCP_CONNECTED) {
        printf("tcp recv data: %s\n", ((char *)tcp + tcp->data_offset));
        nst_wait_by_name("tcp_connect_handle");
    }
}


int distribute_tcp_recv(struct tcp_header *tcp)
{
    print_hex_string((unsigned char *)tcp, sizeof(struct tcp_header), "tcp recv");
    switch(tcp->flags) {
        case TCP_ACK:   
            distribute_tcp_ack(tcp);
            break;

        case TCP_SYN_ACK:   /* 收到第二次握手回复包 */
            if(g_inet_info.tcp_status == TCP_HAND_1) {
                g_inet_info.tcp_status = TCP_HAND_2;
                g_inet_info.seq_num += 1;
                g_inet_info.ack_num = ntohl(tcp->sequence_num) + 1;
                // printf("s.seq = %u s.ack = %u\n", ntohl(tcp->sequence_num), ntohl(tcp->ack_num));
                // printf("c.seq = %u c.ack = %u\n", g_inet_info.seq_num, g_inet_info.ack_num);
                nst_post_by_name("tcp_connect_handle"); /* 唤醒握手线程，继续发送第三个握手包ACK */
            }
            break;
        
    }
}