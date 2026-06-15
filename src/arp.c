#include "../include/arp.h"
#include "../include/utils.h"
#include "../include/netdev.h"

#define ARP_CACHE_SIZE 256
uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t request_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct arp_entry *arp_entry[ARP_CACHE_SIZE];
//In global scope all vals will be NULL at init.

int arp_cache_update(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_entry[i]->hwtype == arphdr->hwtype && arp_entry[i]->ip == arpdata->src_ip) {
            memcpy(arp_entry[i]->mac, arpdata->smac, 6);

            return 1;
        }
    }

    return 0;
}

uint8_t *arp_get_hwaddr(uint32_t sip) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_entry[i]->ip == sip) {
            return arp_entry[i]->mac;
        }
    }

    return NULL;
}

int arp_cache_insert(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (!arp_entry[i]->valid) {
            arp_entry[i]->hwtype = arphdr->hwtype;
            arp_entry[i]->ip = arpdata->src_ip;
            memcpy(arp_entry[i]->mac, arpdata->smac, 6);
            arp_entry[i]->valid = 1;
            arp_entry[i]->timestamp = time(NULL);
            return 0;
        }
    }

    return 1;
}

int arp_reply(struct sk_buff *skb, struct netdev *dev, size_t len) {
    struct arp_hdr *arphdr = (struct arp_hdr *)(skb->data + ETH_HDR_LEN);
    struct arp_ipv4 *arpdata = (struct arp_ipv4 *)arphdr->data;

    int ret = 0;

    memcpy(arpdata->dmac, arpdata->smac, 6);
    memcpy(arpdata->smac, dev->hwaddr, 6);
    arpdata->dest_ip = arpdata->src_ip;
    arpdata->src_ip = dev->addr;
    arpdata_dbg("out ", arpdata);
    arpdata->dest_ip = htonl(arpdata->dest_ip);
    arpdata->src_ip = htonl(arpdata->src_ip);

    arphdr->opcode = ARP_REPLY;
    arphdr_dbg("out ", arphdr);
    arphdr->opcode = htons(arphdr->opcode);
    arphdr->hwtype = htons(arphdr->hwtype);
    arphdr->protype = htons(arphdr->protype);

    return ethernet_reply(arpdata->dmac, arpdata->smac, ARP_ETHERTYPE, dev, skb, len - ETH_HDR_LEN);
}

int arp_recv(struct sk_buff *skb, int len) {
    struct netdev *net_dev;
    int merge = 0;
    struct arp_hdr *arphdr = (struct arp_hdr*)(skb->data + ETH_HDR_LEN);
    arphdr->hwtype = ntohs(arphdr->hwtype);
    arphdr->protype = ntohs(arphdr->protype);
    arphdr->opcode = ntohs(arphdr->opcode);

    arphdr_dbg("in", arphdr);

    if (arphdr->hwtype != ARP_ETHERNET) {
        print_err("ARP: Unsupported HW type\n");
        // goto drop_pkt;
    }
    
    if (arphdr->protype != ARP_IPV4) {
        print_err("ARP: Unsupported protocol\n");
        // goto drop_pkt;
    }

    struct arp_ipv4 *arpdata = (struct arp_ipv4*)(skb->data + ETH_HDR_LEN + ARP_HDR_LEN);

    arpdata->dest_ip = ntohl(arpdata->dest_ip);
    arpdata->src_ip = ntohl(arpdata->src_ip);
    arpdata_dbg("in", arpdata);

    merge = arp_cache_update(arphdr, arpdata);
    if (!(net_dev = netdev_get(arpdata->dest_ip))) {
        print_err("ARP was not for us.\n");
        // goto drop_pkt;
    }

    if ((!merge && arp_cache_insert(arphdr, arpdata)) != 0) {
        print_err("ERR: No free space in ARP translation table\n");
        // goto drop_pkt;
    }

    if (arphdr->opcode == ARP_REQUEST) {
        return arp_reply(skb, net_dev, len);
    } else if (arphdr->opcode == ARP_REPLY) {
        printf("🎉 Got replied to ARP request.\n");
        // goto drop_pkt;
    }

// drop_pkt:
    // free(skb);

    return 0;
}

int arp_request(const uint32_t dip/*in network order*/, struct netdev *dev) {
    struct sk_buff *req_skb = skbuff_alloc(2048);
    // skb_reserve(req_skb, ARP_HDR_LEN + ETH_HDR_LEN + ARP_DATA_LEN);

    struct arp_hdr *arphdr = arp_header(req_skb);
    struct arp_ipv4 *arpdata = (struct arp_ipv4 *)arphdr->data;
    memcpy(arpdata->smac, dev->hwaddr, dev->haddr_len);
    arpdata->src_ip = dev->addr;
    memcpy(arpdata->dmac, request_mac, dev->haddr_len);
    arpdata->dest_ip = ntohl(dip);
    arpdata_dbg("req ", arpdata);
    arpdata->src_ip = htonl(arpdata->src_ip);
    arpdata->dest_ip = htonl(arpdata->dest_ip);
    
    arphdr->hwtype = ARP_ETHERNET;
    arphdr->protype = ARP_IPV4;
    arphdr->hwlen = dev->haddr_len;
    arphdr->prolen = ARP_IPV4_LEN;
    arphdr->opcode = ARP_REQUEST;
    arphdr_dbg("req ", arphdr);
    arphdr->hwtype = htons(arphdr->hwtype);
    arphdr->protype = htons(arphdr->protype);
    arphdr->opcode = htons(arphdr->opcode);
    
    return ethernet_reply(broadcast_mac, arpdata->smac, ARP_ETHERTYPE, dev, req_skb, ARP_HDR_LEN + ARP_DATA_LEN);
}

void arp_cache_init() {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry[i] = calloc(1, sizeof(struct arp_entry));
    }
}

void arp_init() {
    arp_cache_init();
}