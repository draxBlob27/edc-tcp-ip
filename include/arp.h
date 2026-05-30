#ifndef ARP_H
#define ARP_H

#include "syshead.h"
#include "ethernet.h"
#include "time.h"
#include "netdev.h"


#define ARP_HDR_LEN sizeof(struct arp_hdr)
#define ARP_DATA_LEN sizeof(struct arp_ipv4)

#define ARP_ETHERNET    0x0001
#define ARP_ETHERTYPE   0x0806
#define ARP_IPV4        0x0800
#define ARP_REQUEST     0x0001
#define ARP_REPLY       0x0002

struct arp_hdr {
    uint16_t hwtype;     // Hardware type (Ethernet = 1)
    uint16_t protype;     // Protocol type (IPv4 = 0x0800)

    uint8_t  hwlen;      // Hardware address length (MAC = 6)
    uint8_t  prolen;      // Protocol address length (IPv4 = 4)

    uint16_t opcode;      // ARP operation
                        // 1 = request
                        // 2 = reply
    uint8_t data[];
} __attribute__((packed));

struct arp_ipv4 {
    uint8_t smac[6];
    uint32_t src_ip;
    uint8_t dmac[6];
    uint32_t dest_ip;
} __attribute__((packed));

struct arp_entry {
    uint16_t hwtype;
    uint32_t ip;
    uint8_t mac[6];
    time_t timestamp;
    int valid;
};

void arp_recv(void *buffer, int len);
void arp_reply(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata, struct netdev *dev);
int arp_cache_update(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata);
int arp_cache_insert(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata);

#endif //ARP_H