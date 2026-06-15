#include "../include/ipv4.h"
#include "../include/icmpv4.h"
#include "../include/arp.h"
#include "../include/utils.h"

uint16_t internet_checksum(void *addr, size_t count, uint64_t st_sum) {
    //assumes underlying data is in network order.
    uint64_t csum = st_sum;
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

void ipv4_recv(struct sk_buff *skb, size_t len) {
    struct ipv4_hdr *ipv4hdr = ipv4_header(skb); //network order
    struct netdev *net_dev;
    
    if (ipv4hdr->version != IPV4) {
        print_err("IPV4: Protocol not supported.\n");
        return;
    }

    if (ipv4hdr->ttl <= 0) {
        print_err("IPV4: Hop limit exhausted\n");
        return;
    }

    if (!(net_dev = netdev_get(ntohl(ipv4hdr->dest_addr)))) {
        print_err("IPV4: Datagram not for us.\n");
        return;
    }

    uint16_t recv_csum = internet_checksum(ipv4hdr, ipv4hdr->ihl * 4, 0);
    if (recv_csum != 0) {
        print_err("IPV4: Datagram invalidated.\n");
        return;
    }

    ipv4hdr->len = ntohs(ipv4hdr->len);
    ipv4hdr->id = ntohs(ipv4hdr->id);
    ipv4hdr->flags_and_frag_offset = ntohs(ipv4hdr->flags_and_frag_offset);
    ipv4hdr->hdr_csum = ntohs(ipv4hdr->hdr_csum);
    ipv4hdr->src_addr = ntohl(ipv4hdr->src_addr);
    ipv4hdr->dest_addr = ntohl(ipv4hdr->dest_addr);

    ipv4hdr_dbg("in ", ipv4hdr);

    ipv4hdr->len = htons(ipv4hdr->len);
    ipv4hdr->id = htons(ipv4hdr->id);
    ipv4hdr->flags_and_frag_offset = htons(ipv4hdr->flags_and_frag_offset);
    ipv4hdr->hdr_csum = htons(ipv4hdr->hdr_csum);
    ipv4hdr->src_addr = htonl(ipv4hdr->src_addr);
    ipv4hdr->dest_addr = htonl(ipv4hdr->dest_addr);

    if (ipv4hdr->protocol == ICMPV4) {
        icmpv4_recv(skb, len, net_dev);
    } else {
        print_err("IPV4: Not an ICMP msg.\n");
    }
}

int ipv4_reply(uint32_t dip/*in netwrok order*/, uint8_t protocol, struct sk_buff *skb, size_t len, struct netdev *dev) {
    struct ipv4_hdr *ipv4hdr = ipv4_header(skb);

    ipv4hdr->ihl = 5;
    ipv4hdr->version = IPV4;
    ipv4hdr->tos = 0;
    ipv4hdr->len = IPV4_HDR_LEN + len; //host order
    ipv4hdr->id = 0x0101; //host order
    ipv4hdr->flags_and_frag_offset = 0x4000; //host order
    ipv4hdr->ttl = 64;
    ipv4hdr->hdr_csum = 0;
    ipv4hdr->src_addr = dev->addr;//in host order
    ipv4hdr->dest_addr = ntohl(dip); //in host order
    ipv4hdr->protocol = protocol;

    ipv4hdr_dbg("out ", ipv4hdr);

    ipv4hdr->len = htons(ipv4hdr->len);
    ipv4hdr->id = htons(ipv4hdr->id);
    ipv4hdr->flags_and_frag_offset = htons(ipv4hdr->flags_and_frag_offset);
    ipv4hdr->hdr_csum = htons(ipv4hdr->hdr_csum);
    ipv4hdr->src_addr = htonl(ipv4hdr->src_addr);//in netowrk order
    ipv4hdr->dest_addr = htonl(ipv4hdr->dest_addr); //in network order

    ipv4hdr->hdr_csum = 0;
    ipv4hdr->hdr_csum = internet_checksum(ipv4hdr, ipv4hdr->ihl * 4, 0); //in network order;

    uint8_t *dmac = arp_get_hwaddr(ntohl(dip));
    if (!dmac) {
        if (arp_request(dip, dev) != -1) {
            printf("IPV4: Sent ARP Request.\n");
        };
        
        printf("Retry again\n");
        return -1;
    }

    return ethernet_reply(dmac, dev->hwaddr, ARP_IPV4, dev, skb, IPV4_HDR_LEN + len);
}