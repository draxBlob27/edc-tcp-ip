#ifndef ARP_H
#define ARP_H

#include "syshead.h"

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
    uint8_t src_ip[4];
    uint8_t dmac[6];
    uint8_t dest_ip[4];
} __attribute__((packed));

#endif //ARP_H