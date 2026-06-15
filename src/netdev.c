#include "../include/netdev.h"
#include "../include/arp.h"
#include "../include/ethernet.h"
#include "../include/utils.h"
#include "../include/tuntap_alloc.h"

struct netdev *my_dev;

struct netdev *netdev_alloc(char *if_name, char *addr, char *hwaddr, uint32_t mtu) {
    struct netdev *dev = malloc(sizeof(struct netdev));
    dev->fd = tun_alloc(if_name, IFF_TAP | IFF_NO_PI);
    if(dev->fd < 0){
      perror("Attaching interface\n");
      exit(1);
    }

    dev->haddr_len = 6;
    parse_ip(addr, &dev->addr);

    dev->addr = ntohl(dev->addr);
    sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dev->hwaddr[0],
                                                    &dev->hwaddr[1],
                                                    &dev->hwaddr[2],
                                                    &dev->hwaddr[3],
                                                    &dev->hwaddr[4],
                                                    &dev->hwaddr[5]);
    dev->mtu = mtu;
    return dev;
}

struct netdev *netdev_get(uint32_t sip) {
    // printf("%u.%u.%u.%u", \
    //     (my_dev->addr >> 24) & 0xff, \
    //     (my_dev->addr >> 16) & 0xff,\
    //     (my_dev->addr >> 8)  & 0xff,\
    //     my_dev->addr & 0xff);
    if (my_dev->addr == sip) {
        return my_dev;
    } else {
        return NULL;
    }
}

int netdev_transmit(struct sk_buff *skb, struct netdev *dev) {
    tun_write(skb->data, skb->len, dev->fd);
    // free_skb(skb);
}

int netdev_init(char *if_name, char *addr, char *hwaddr) {
    my_dev = netdev_alloc(if_name, addr, hwaddr, 1500);
    return my_dev->fd;
}