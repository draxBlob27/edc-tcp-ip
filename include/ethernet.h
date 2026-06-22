#ifndef ETHERNET_H
#define ETHERNET_H
#define DEBUG_ETH

#include "syshead.h"
#include "skbuff.h"
#include "netdev.h"

#define ARP_ETHERTYPE   0x0806
#define ETH_HDR_LEN sizeof(struct eth_hdr)

#ifdef DEBUG_ETH
#define ethhdr_dbg(str, hdr) \
    do { \
        printf("eth hdr: "str" dmac: %02x:%02x:%02x:%02x:%02x:%02x " \
        "smac: %02x:%02x:%02x:%02x:%02x:%02x " \
        "ethertype: 0x%04x\n", hdr->dst_mac[0], hdr->dst_mac[1], hdr->dst_mac[2], \
        hdr->dst_mac[3], hdr->dst_mac[4], hdr->dst_mac[5], hdr->src_mac[0], \
        hdr->src_mac[1], hdr->src_mac[2], hdr->src_mac[3], hdr->src_mac[4], \
        hdr->src_mac[5], hdr->ethertype); \
    } while(0) 
#else
#define ethhdr_dbg(str, hdr)
#endif

struct eth_hdr {
    uint8_t dst_mac[6]; 
    uint8_t src_mac[6]; 
    uint16_t ethertype; 
    uint8_t payload[];
} __attribute__((packed));

void parse_ethernet(struct sk_buff *skb, int nread);
int ethernet_reply(uint8_t *dst_mac, uint8_t *src_mac, uint16_t ethertype,\
     struct netdev *dev, struct sk_buff *skb, size_t len);

static inline struct eth_hdr *eth_Header(struct sk_buff *skb) {
    return (struct eth_hdr *)skb->data;
}

#endif //ETHERNET_H