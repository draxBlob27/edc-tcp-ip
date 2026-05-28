#ifndef ARP_H
#define ARP_H

#include "syshead.h"
#include "ehternet.h"
#include "time.h"

const int ARP_REQUEST = 1;
const int ARP_REPLY = 2;

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

struct arp_entry {
    uint32_t ip;
    uint8_t mac[6];
    time_t timestamp;
    int valid;
};

void arp_recv(void *buffer, int len) {
    uint8_t *ptr = (uint8_t *)buffer;

    ptr += sizeof(struct eth_hdr);
    struct arp_hdr *arp = (struct arp_hdr*)ptr;

    ptr += sizeof(struct arp_hdr);
    struct arp_ipv4 *ip = (struct arp_ipv4*)ptr;


    if (ntohs(arp->opcode) == ARP_REQUEST) {//REQUEST
        arp_reply();
        return;
    } else if 
}

void arp_reply() {

}

uint8_t arp_cache_lookup(uint32_t ip) {

}

void arp_cache_insert(uint32_t ip, uint8_t mac[6]) {

}

void arp_cache_update() {

}

#endif //ARP_H