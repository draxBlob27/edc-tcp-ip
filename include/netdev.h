#ifndef NETDEV_H
#define NETDEV_H

#include "syshead.h"

struct netdev {
    uint32_t addr;
    uint8_t addr_len;
    uint8_t hwaddr[6];
    uint32_t mtu;
};

void netdev_alloc(char *addr, char *hwaddr, uint32_t mtu);
void netdev_init(char *addr, char *hwaddr);
struct netdev *netdev_get(uint32_t src_ip);


#endif //NETDEV_H