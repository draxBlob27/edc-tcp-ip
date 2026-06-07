#include "../include/icmpv4.h"
#include "../include/ipv4.h"
#include "../include/ethernet.h"
#include "../include/utils.h"

void icmpv4_recv(struct sk_buff *skb, size_t len, struct netdev *dev) {
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)(skb->data + ETH_HDR_LEN + IPV4_HDR_LEN);

    icmpv4hdr_dbg("in ", icmpv4hdr); //network order

    uint16_t csum = internet_checksum(icmpv4hdr, skb->len - ETH_HDR_LEN - IPV4_HDR_LEN);
    if (csum != 0) {
        print_err("ICMP: Packet corrupted\n");
        return;
    }

    if (icmpv4hdr->type == ECHO_REQUEST) {
        icmpv4_reply(skb, len, dev);
    }
}

void icmpv4_reply(struct sk_buff *skb, size_t len, struct netdev *dev) {
    //this contains ipv4 hdr in host order
    struct ipv4_hdr *ipv4hdr = (struct ipv4_hdr *)(skb->data);
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)(ipv4hdr->data); //in network order

    icmpv4hdr->type = ECHO_REPLY;
    icmpv4hdr->checksum = 0;
    icmpv4hdr->checksum = internet_checksum(icmpv4hdr, skb->len - ETH_HDR_LEN - IPV4_HDR_LEN);

    icmpv4hdr_dbg("out ", icmpv4hdr);

    int ret = ipv4_reply(ipv4hdr->src_addr, ICMPV4, skb, len, dev);
    if (ret == -1) {
        return;
    }
}