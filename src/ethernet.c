#include "../include/ethernet.h"
#include "../include/arp.h"

void parse_ethernet(struct sk_buff *skb, int nread) {
    struct eth_hdr *ethhdr = (struct eth_hdr *)skb->data;

    struct arp_hdr *arphdr = (struct arp_hdr *)(skb->data + ETH_HDR_LEN);
    ethhdr->ethertype = ntohs(ethhdr->ethertype);

    ethhdr_dbg("in ", ethhdr);

    if (ethhdr->ethertype == ARP_ETHERTYPE) {//ARPING
        arp_recv(skb, nread);
        
    } else if (ethhdr->ethertype == 0x0800) {//IPv4
        printf("Ipv4 address\n");
    } else {
        printf("Ipv6 or corrupted\n");
    }

    free_skb(skb);
}

int ethernet_reply(uint8_t *dst_mac, uint8_t *src_mac, uint16_t ethertype,\
     struct netdev *dev, struct sk_buff *skb, size_t len) {
        struct eth_hdr *ethhdr = (struct eth_hdr *)skb->data;
        memcpy(ethhdr->dst_mac, dst_mac, 6);
        memcpy(ethhdr->src_mac, src_mac, 6);
        ethhdr->ethertype = ethertype; //in host order

        ethhdr_dbg("out ", ethhdr);
        ethhdr->ethertype = htons(ethhdr->ethertype);
        int ret = netdev_transmit(skb, dev);
    }