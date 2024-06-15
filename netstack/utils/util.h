#ifndef __UTIL_H__
#define __UTIL_H__
#include "common.h"

const char *family_to_string(int family);   /* 协议族-》字符串形式 */
const char *ether_id_to_string(int ethid);  /* 以太网协议的type字段-》字符串形式 */
in_addr_t get_inet_ip(const char *name, sa_family_t _family); //获取指定网卡、指定协议族的ip地址
int get_eth_mac(const char *name, void *mac); /* 获取指定网卡的mac地址 */
#endif /* __UTIL_H__ */
