#include "common.h"
#include "util.h"
#include "sa_info.h"
#include "logger.h"
#include "protocol_cache.h"
#include "arp.h"
#include "thread.h"
#include "udp.h"
#include "tcp.h"
#include <cstddef>
#include <cstring>
#include <unistd.h>

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
    } else if(strcmp(command, "tcpconect") == 0) {
        if(g_inet_info.tcp_status != TCP_CONNECTED) {
            nst_create(&tcp_connect_thread, tcp_connect_handle, "tcp_connect_handle", &tcp_connect_thread);
            nst_destroy(&tcp_connect_thread);
            if(g_inet_info.tcp_status != TCP_CONNECTED) {
                tcp_init_seq(&g_inet_info);
                return;
            }
            printf("tcp connected success...\n");
        } else {
            printf("tcp connected already...\n");
        }
    } else if(strcmp(command, "tcpsend") == 0) {
        if(g_inet_info.tcp_status == TCP_CONNECTED) {
            tcp_send(&g_inet_info, (char *)message, strlen(message));
        }
    } else {
        printf("Unknown command: %s\n", command);
    }
}

void *udp_recv_handle(void *priv)
{
    char data[MTU];
    size_t len = 0;
    while(1) {
        len = udp_recv(&g_inet_info, data, MTU);
        data[len+1] = '\0';
        std::cout << "-------udp recv: " <<  data << std::endl;
    }
}

void *tcp_data_recv_handle(void *priv)
{
    while(g_inet_info.tcp_status != TCP_CONNECTED) {
        usleep(1000*50);
    }

    // char data[MTU];
    // size_t len = 0;
    // while(1) {
    //     len = udp_recv(&g_inet_info, data, MTU);
    //     data[len+1] = '\0';
    //     std::cout << "-------udp recv: " <<  data << std::endl;
    // }
    while(1);
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
    
    nst_destroy(&icmp_req_thread);
    while(g_inet_info.ping_success == false) { /* 等待网络ping通了 */
        usleep(1000*50);
    }

    nst_create(&udp_recv_thread, udp_recv_handle, "udp_recv_handle", &udp_recv_thread);
    nst_create(&tcp_data_recv_thread, tcp_data_recv_handle, "tcp_data_recv_handle", &tcp_data_recv_thread);

    tcp_init_seq(&g_inet_info); /* 初始化tcp的seq和ack值 */


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
    nst_destroy(&monitor_thread);
}
