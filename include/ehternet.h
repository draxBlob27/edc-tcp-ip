#ifndef ETHERNET_H
#define ETHERNET_H

#include "syshead.h"

#define ETH_HDR_LEN sizeof(struct eth_hdr)

struct eth_hdr {
    uint8_t dst_mac[6]; 
    uint8_t src_mac[6]; 
    uint16_t ethertype; 
    uint8_t payload[];
} __attribute__((packed));

#endif //ETHERNET_H