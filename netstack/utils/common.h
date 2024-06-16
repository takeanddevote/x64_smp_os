#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "pcap.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <thread.h>
#include<net/ethernet.h>
#include<socket.h>
#include <pthread.h>

extern ns_thread_t icmp_req_thread;
extern ns_thread_t monitor_thread;
extern ns_thread_t udp_recv_thread;
extern ns_thread_t tcp_data_recv_thread;
extern ns_thread_t tcp_connect_thread;
void *icmp_req_handle(void *priv);
void *monitor_handle(void *priv);
void *udp_recv_handle(void *priv);
void *tcp_connect_handle(void *priv);


#endif /* __COMMON_H__ */
