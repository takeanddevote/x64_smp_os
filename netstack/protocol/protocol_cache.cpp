#include "protocol_cache.h"
#include "logger.h"
inet_info_t g_inet_info;


void print_inet_info()
{
    struct in_addr in_addr;
    in_addr.s_addr = g_inet_info.local_ip;
    printf("local ip %s.\n", inet_ntoa(in_addr));
    print_hex_string((unsigned char *)g_inet_info.local_mac, ETH_ALEN, "local mac");

    in_addr.s_addr = g_inet_info.remote_ip;
    printf("remote ip %s.\n", inet_ntoa(in_addr));
    print_hex_string((unsigned char *)g_inet_info.remote_mac, ETH_ALEN, "remote mac");
    fflush(stdout);
}