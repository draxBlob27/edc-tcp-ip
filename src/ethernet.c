#include "../include/ethernet.h"
#include "../include/arp.h"

void parse_ethernet(struct sk_buff *skb, int nread) {
    struct eth_hdr *ethhdr = (struct eth_hdr *)skb->data;
    skb_pull(skb, ETH_HDR_LEN);

    struct arp_hdr *arphdr = (struct arp_hdr *)skb->data;
    ethhdr->ethertype = ntohs(ethhdr->ethertype);

    ethhdr_dbg("in ", ethhdr);

    if (ethhdr->ethertype == ARP_ETHERTYPE) {//ARPING
        skb_pull(skb, ARP_HDR_LEN);
        arp_recv(skb, nread);
        
    } else if (ethhdr->ethertype == 0x0800) {//IPv4
        printf("Ipv4 address\n");
    } else {
        printf("Ipv6 or corrupted\n");
    }

    free_skb(skb);
}