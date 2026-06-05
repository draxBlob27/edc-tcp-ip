#include "../include/ipv4.h"
#include "../include/icmpv4.h"
#include "../include/ethernet.h"
#include "../include/netdev.h"



uint16_t internet_checksum(void *addr, size_t count) {
    uint64_t csum = 0;
    uint8_t *p = addr;
    while(count > 1) {
        csum += *(uint16_t *)p;
        p += 2;
        count -= 2;
    }

    if (count > 0) {
        csum += *(uint8_t *)p;
    }

    csum = (csum & 0xffffffff) + (csum >> 32);
    csum = (csum & 0xffff) + (csum >> 16);
    csum = (csum & 0xffff) + (csum >> 16);

    return (uint16_t)(~csum);
}

void ipv4_recv(char *buffer, size_t len) {
    struct ipv4_hdr *ipv4hdr = (struct ipv4_hdr *)(buffer + ETH_HDR_LEN);
    struct netdev *net_dev;
    
    if (ipv4hdr->version != IPV4) {
        print_err("IPV4: Protocol not supported.\n");
    }

    if (ipv4hdr->ttl <= 0) {
        print_err("IPV4: Hop limit exhausted\n");
    }

    if (!(net_dev = netdev_get(ipv4hdr->dest_addr))) {
        print_err("IPV4: Datagram not for us.\n");
    }

    uint16_t recv_csum = internet_checksum(ipv4hdr, ipv4hdr->ihl * 4);
    if (recv_csum != 0) {
        print_err("IPV4: Datagram invalidated.\n");
        return;
    }

    if (ipv4hdr->protocol == ICMPV4) {
        icmpv4_recv(buffer, len);
        return;
    } else {
        print_err("IPV4: Not an ICMP msg.\n");
    }
}

void ipv4_send(uint32_t dip, uint8_t protocol, char *buffer, size_t len) {

}