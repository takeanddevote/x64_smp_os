#include "util.h"


const char *ether_id_to_string(int ethid)
{
    return (ethid == ETHERTYPE_IP) ? "IPV4" : (ethid == ETHERTYPE_IPV6) ? "IPV6" :  \
           (ethid == ETHERTYPE_ARP) ? "ARP" : "????";
}

const char *family_to_string(int family)
{
    return (family == AF_INET) ? "AF_INET" : (family == AF_INET6) ? "AF_INET6" : (family == AF_PACKET) ? "AF_PACKET" : "???";
}

in_addr_t get_inet_ip(const char *name, sa_family_t _family) 
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    in_addr_t ret = -1;

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;

        if (family == _family && !strcmp(ifa->ifa_name, name)) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                            sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                ret = -1;
                break;
            }
            struct in_addr inp;
            inet_aton(host, &inp);
            ret = inp.s_addr;
        }
    }

    freeifaddrs(ifaddr);
    return ret;
}

int get_eth_mac(const char *name, void *mac)
{
    struct ifreq ifr;   /* 网络接口请求数据结构 */

    int fd = socket(AF_INET, SOCK_DGRAM, 0);    /* 创建ipv4、udp通信套接字，用于ioctl获取mac地址 */
    if(fd < 0) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);   /* 设置网卡名字 */
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if(ioctl(fd, SIOCGIFHWADDR, &ifr)) { /* 获取指定网卡名字的mac地址 */
        perror("ioctl");
        close(fd);
        return -1;
    }
    memcpy(mac, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
    close(fd);

    return 0;
}

uint16_t ip_checksum(uint16_t *data, int length) 
{
    uint32_t sum = 0;

    while (length > 1) {
        sum += *data++;
        length -= 2;
    }

    if (length == 1) {
        sum += *((uint8_t*) data);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t) ~sum;
}