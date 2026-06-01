#ifndef NETDEV_H
#define NETDEV_H

#include "syshead.h"
#include "tuntap_alloc.h"

struct netdev {
    uint32_t addr;
    uint8_t haddr_len;
    uint8_t hwaddr[6];
    uint32_t mtu;
    int fd;
};

struct netdev *netdev_alloc(char *if_name, char *addr, char *hwaddr, uint32_t mtu);
int netdev_init(char *if_name, char *addr, char *hwaddr);
struct netdev *netdev_get(uint32_t src_ip);

#endif //NETDEV_H