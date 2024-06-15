#include "common.h"
#include "logger.h"
#include "thread.h"
#include "protocol_cache.h"
#include "arp.h"



ns_thread_t icmp_req_thread;
ns_thread_t monitor_thread;

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
    nst_wait(&monitor_thread);  /* 等待ARP回复 */

    print_inet_info();
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
                nst_post(&monitor_thread);
            }
            break;
        case ETHERTYPE_IP:
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
    if(pcap_loop(handle, -1, packet_recv, NULL)) {  /* 开始抓包，无限次 */
        fprintf(stderr, "pcap_loop failed: %s\n", pcap_geterr(handle));
    }
    pcap_close(handle);
    return NULL;
}