#include "sa_info.h"
#include "util.h"
#include "logger.h"


void print_sys_sa_info() 
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;

        printf("name %s family %s.\n", ifa->ifa_name, family_to_string(family));
        print_hex_string((unsigned char *)ifa->ifa_addr->sa_data, 14, "sadata");
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                            sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\t\taddress: <%s>\n", host);

        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;

            printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                    "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                    stats->tx_packets, stats->rx_packets,
                    stats->tx_bytes, stats->rx_bytes);
        }
    }


    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);
}

int print_mac_address(void)
{
    unsigned char mac[6];

    struct ifreq ifr;   /* 网络接口请求数据结构 */

    int fd = socket(AF_INET, SOCK_DGRAM, 0);    /* 创建ipv4、udp通信套接字，用于ioctl获取mac地址 */
    if(fd < 0) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, "ens33", IFNAMSIZ - 1);   /* 设置网卡名字 */
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if(ioctl(fd, SIOCGIFHWADDR, &ifr)) { /* 获取指定网卡名字的mac地址 */
        perror("ioctl");
        close(fd);
        return -1;
    }

    print_hex_string((unsigned char *)ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN, "mac");

    close(fd);
}