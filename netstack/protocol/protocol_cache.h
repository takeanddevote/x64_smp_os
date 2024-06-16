#ifndef __PROTOCOL_CACHE_H__
#define __PROTOCOL_CACHE_H__
#include "common.h"
#include <cstddef>
#include <mutex>

extern "C" {
#define MTU 1500


enum tcp_state {
    TCP_CLOSED = 0,     /* 关闭状态 */
    TCP_HAND_1 = 1,     /* 发送了第一次握手SYN包*/
    TCP_HAND_2 = 2,     /* 接受了第二次握手ACK+SYN */
    TCP_CONNECTED = 3,  /* 发送了第三次握手ACK包 */
};


typedef struct {
    char name[32];                      /* 网卡名字 */
    in_addr_t local_ip;                 /* 本地IP地址 */
    in_addr_t remote_ip;                /* 远端IP地址 */
    u_int8_t local_mac[ETH_ALEN];       /* 本地mac地址 */
    u_int8_t remote_mac[ETH_ALEN];      /* 远端mac地址 */
    sa_family_t family;                 /* 协议族 */
    pcap_t *handle;                     /* pcap句柄 */

    size_t local_port;                  /* 本地端口号 */
    size_t remote_port;                 /* 远端端口号 */

    bool ping_success;      /* 网络已经ping通了？ */

    /* tcp/udp包数据 */
    std::mutex datalock;
    char data[MTU];
    size_t data_valid;

    /* tcp的seq和ack值 */
    std::mutex tcplock;
    uint32_t seq_num;
    uint32_t ack_num;
    enum tcp_state tcp_status;

} inet_info_t;

extern inet_info_t g_inet_info;
void print_inet_info();


}

#endif /* __PROTOCOL_CACHE_H__ */
