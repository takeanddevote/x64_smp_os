#include "common.h"
#include "util.h"
#include "sa_info.h"
#include "logger.h"
#include "protocol_cache.h"
#include "arp.h"
#include "thread.h"
#include "udp.h"
#include <cstddef>
#include <cstring>

#define INET_NAME       "ens38"
#define REMOTE_IP       "192.168.133.138"
#define LOCAL_PORT      1234
#define REMOTE_PORT     8080

void *priv;
void handle_sigint(int sig) {
    // pcap_close(priv);
    debugsit
}

#define MAX_INPUT_SIZE 128

void performAction(const char *command, const char *message) {
    if (strcmp(command, "udp") == 0) {
        udp_send(&g_inet_info, (void *)message, (size_t)strlen(message));
    } else if(strcmp(command, "tcp") == 0) {
        
    } else {
        printf("Unknown command: %s\n", command);
    }
}

int main()
{
    char errbuf[PCAP_ERRBUF_SIZE];  /* 用于pacap错误时，返回错误信息 */
    signal(SIGKILL, handle_sigint);

    strcpy(g_inet_info.name, INET_NAME);  /* 监控网卡名字 */
    g_inet_info.local_port = LOCAL_PORT;  /* 本地端口号 */
    g_inet_info.remote_port = REMOTE_PORT; /* 远端端口号 */

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

    nst_create(&monitor_thread, monitor_handle, "monitor_handle", &monitor_thread);
    nst_create(&icmp_req_thread, icmp_req_handle, "icmp_req_handle", &icmp_req_thread); /* icmp获取远端信息 */

    char input[MAX_INPUT_SIZE];
    char command[10];    // 用于存储第一个参数
    char message[MAX_INPUT_SIZE];  // 用于存储第二个参数
    while (1) {
        fgets(input, MAX_INPUT_SIZE, stdin);  // 获取用户输入

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "q") == 0) {
            printf("Exiting the program...\n");
            break;
        }

        // 使用 sscanf 解析输入
        int numArgs = sscanf(input, "%s %[^\n]", command, message);
        
        // 检查是否输入了两个参数
        if (numArgs < 2) {
            printf("Invalid input. Please enter a command followed by a message.\n");
            continue;
        }

        // 根据输入执行相应操作
        performAction(command, message);
    }

    nst_destroy(&icmp_req_thread);
    nst_destroy(&monitor_thread);
}
