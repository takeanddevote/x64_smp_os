#include "common.h"
#include "logger.h"
#include "thread.h"
#include "protocol_cache.h"
#include "arp.h"
#include "icmp.h"
#include "ip.h"
#include "udp.h"
#include "tcp.h"



ns_thread_t icmp_req_thread;
ns_thread_t monitor_thread;
ns_thread_t udp_recv_thread;
ns_thread_t tcp_data_recv_thread;
ns_thread_t tcp_connect_thread;

// void *arp_request(void *priv)
// {
//     /* 发送ARP请求包 */
// }


void *icmp_req_handle(void *priv)
{
    sleep(1);

    // ns_thread_t arp_thread;
    // nst_create(&arp_thread, arp_request, &arp_thread); /* 创建线程发送arp请求，获取远端mac地址 */

    send_arp_req(&g_inet_info);     /* 发送arp请求包 */
    nst_wait_by_name("monitor_handle");  /* 等待ARP回复 */

    // print_inet_info();

    send_icmp_echo_request_packet(&g_inet_info);    /* 发送icmp回显请求，测试连通性 */
    nst_wait_by_name("monitor_handle");                    /* 等待回显回复 */

    std::cout << "ping " << (g_inet_info.ping_success ? "success" : "false") << std::endl;
}


static void packet_recv(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{   /* user-私有数据 pkthdr-数据包头部信息 packet-原始数据包，不包括前导码 */
    struct ether_header *header = reinterpret_cast<struct ether_header *>(const_cast<u_char *>(packet));
    // print_hex_string(packet, 13, "predao");
    int ret = 0;
    switch(ntohs(header->ether_type)) {
        case ETHERTYPE_ARP: /* arp reply */
            ret = deal_arp_reply(&g_inet_info, packet);
            if(!ret) {
                nst_post_by_name("icmp_req_handle");
            }
            break;
        case ETHERTYPE_IP:
            ret = distribute_ip_reply(packet);
            break;

        default:
            break;
    }
}

void *monitor_handle(void *priv)
{
    char errbuf[PCAP_ERRBUF_SIZE];  /* 用于pacap错误时，返回错误信息 */
    pcap_t *handle;
    handle = pcap_open_live(g_inet_info.name, 65535, 1, 1000, errbuf); /* 创建抓包句柄，需要root权限 */
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", g_inet_info.name, errbuf);
        return NULL;
    }
    g_inet_info.handle = handle;
    if(pcap_loop(handle, -1, packet_recv, NULL)) {  /* 开始抓包，无限次; packet_recv 是当前线程回调的，而不是新创建一个线程 */
        fprintf(stderr, "pcap_loop failed: %s\n", pcap_geterr(handle));
    }
    pcap_close(handle);
    return NULL;
}

void *tcp_connect_handle(void *priv)
{
    printf("c.seq = %u c.ack = %u\n", g_inet_info.seq_num, g_inet_info.ack_num);
    tcp_send_control(&g_inet_info, TCP_SYN);     /* 第一次握手，发送SYN包 */
    g_inet_info.tcp_status = TCP_HAND_1;

    nst_wait_by_name("monitor_handle");               /* 等待服务端回复的第二次握手包，SYN+ACK */
    if(g_inet_info.tcp_status == TCP_HAND_2) {
        printf("c.seq = %u c.ack = %u\n", g_inet_info.seq_num, g_inet_info.ack_num);
        tcp_send_control(&g_inet_info, TCP_ACK);     /* 第三次握手，发送ACK包 */
        g_inet_info.tcp_status = TCP_CONNECTED;
    }

    std::cout << "tcp status " << g_inet_info.tcp_status << std::endl;
}