#include "../include/icmpv4.h"
#include "../include/ipv4.h"
#include "../include/ethernet.h"
#include "../include/utils.h"

void icmpv4_recv(struct sk_buff *skb, size_t len, struct netdev *dev) {
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)(skb->data + ETH_HDR_LEN + IPV4_HDR_LEN);

    icmpv4hdr->checksum = ntohs(icmpv4hdr->checksum);
    icmpv4hdr->identifier = ntohs(icmpv4hdr->identifier);
    icmpv4hdr->sequence_no = ntohs(icmpv4hdr->sequence_no);

    icmpv4hdr_dbg("in ", icmpv4hdr); //network order

    icmpv4hdr->checksum = htons(icmpv4hdr->checksum);
    icmpv4hdr->identifier = htons(icmpv4hdr->identifier);
    icmpv4hdr->sequence_no = htons(icmpv4hdr->sequence_no);

    uint16_t csum = internet_checksum(icmpv4hdr, skb->len - ETH_HDR_LEN - IPV4_HDR_LEN, 0);
    if (csum != 0) {
        print_err("ICMP: Packet corrupted\n");
        return;
    }

    if (icmpv4hdr->type == ECHO_REQUEST) {
        icmpv4_reply(skb, len, dev);
    } else if (icmpv4hdr->type == ECHO_REPLY) {
        printf("🎉 Got replied to ping.\n");
    }
}

void icmpv4_reply(struct sk_buff *skb, size_t len, struct netdev *dev) {
    //this contains ipv4 hdr in network order
    struct ipv4_hdr *ipv4hdr = (struct ipv4_hdr *)(skb->data + ETH_HDR_LEN);
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)(ipv4hdr->data); //in network order

    icmpv4hdr->type = ECHO_REPLY;
    icmpv4hdr->checksum = 0;
    icmpv4hdr->checksum = internet_checksum(icmpv4hdr, skb->len - ETH_HDR_LEN - IPV4_HDR_LEN, 0);

    icmpv4hdr->checksum = ntohs(icmpv4hdr->checksum);
    icmpv4hdr->identifier = ntohs(icmpv4hdr->identifier);
    icmpv4hdr->sequence_no = ntohs(icmpv4hdr->sequence_no);

    icmpv4hdr_dbg("out ", icmpv4hdr);

    icmpv4hdr->checksum = htons(icmpv4hdr->checksum);
    icmpv4hdr->identifier = htons(icmpv4hdr->identifier);
    icmpv4hdr->sequence_no = htons(icmpv4hdr->sequence_no);

    int ret = ipv4_reply(ipv4hdr->src_addr, ICMPV4, skb, len, dev);
    if (ret == -1) {
        return;
    }
}

int icmpv4_request(const uint32_t dip/*in network order*/, struct netdev *dev) {
    struct sk_buff *req_skb = skbuff_alloc(2048);
    skb_reserve(req_skb, IPV4_HDR_LEN + ETH_HDR_LEN + ICMPV4_HDR_LEN + 56);

    skb_push(req_skb, ICMPV4_HDR_LEN + 56);
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)req_skb->data;
    icmpv4hdr->type = ECHO_REQUEST;
    icmpv4hdr->code = 0;
    icmpv4hdr->identifier = 0x1234;
    icmpv4hdr->sequence_no = 0x0001;
    memset(icmpv4hdr->data, 0xAB, 56);
    icmpv4hdr->checksum = 0;

    icmpv4hdr_dbg("out ", icmpv4hdr);

    icmpv4hdr->checksum = htons(icmpv4hdr->checksum);
    icmpv4hdr->identifier = htons(icmpv4hdr->identifier);
    icmpv4hdr->sequence_no = htons(icmpv4hdr->sequence_no);

    icmpv4hdr->checksum = internet_checksum(icmpv4hdr, 64, 0);

    skb_push(req_skb, IPV4_HDR_LEN);
    return ipv4_reply(dip, ICMPV4, req_skb, ICMPV4_HDR_LEN + 56, dev);
}