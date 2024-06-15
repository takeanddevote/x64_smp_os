#include "common.h"
#include "util.h"
#include "sa_info.h"
#include "logger.h"
#include "protocol_cache.h"
#include "arp.h"
#include "thread.h"

#define INET_NAME   "ens38"
#define REMOTE_IP   "192.168.133.138"

void *priv;
void handle_sigint(int sig) {
    // pcap_close(priv);
    debugsit
}



int main()
{
    char errbuf[PCAP_ERRBUF_SIZE];  /* 用于pacap错误时，返回错误信息 */
    signal(SIGKILL, handle_sigint);

    strcpy(g_inet_info.name, INET_NAME);  /* 监控网卡名字 */

    /* 查找指定网卡是否可用 */
    pcap_if_t *devs, *dev;
    if (pcap_findalldevs(&devs, errbuf)) {
        fprintf(stderr, "lookup no device: %s\n", errbuf);
        return 1;
    }

    char *device = NULL;
    for(dev = devs; dev; dev = dev->next) {
        if(!strcmp(g_inet_info.name, dev->name)) {
            device = dev->name;
            break;
        }
    }

    if(!device) {
        fprintf(stderr, "lookup no valid device.\n");
        return 1;
    }

    printf("found %s.\n", device);
    pcap_freealldevs(devs);

    /* 远端ip地址 */
    struct in_addr inp;
    inet_aton(REMOTE_IP, &inp);
    g_inet_info.remote_ip = inp.s_addr;

    /* 初始化本地网卡信息 */
    g_inet_info.family = AF_INET;   /* 协议族 */
    g_inet_info.local_ip = get_inet_ip(g_inet_info.name, AF_INET);  /* 获取网卡ip地址 */
    get_eth_mac(g_inet_info.name, g_inet_info.local_mac);              /* 获取网卡mac地址 */

    nst_create(&monitor_thread, monitor_handle, &monitor_thread); 
    nst_create(&icmp_req_thread, icmp_req_handle, &icmp_req_thread); /* icmp获取远端信息 */

    nst_destroy(&icmp_req_thread);
    nst_destroy(&monitor_thread);
}
