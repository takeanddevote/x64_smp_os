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

int main()
{
    char errbuf[128];
    char *devie = pcap_lookupdev(errbuf);
    printf("%s.\n", devie);
    printf("hello world.\n");

    // pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf);

}
