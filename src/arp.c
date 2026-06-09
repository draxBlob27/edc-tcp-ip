#include "../include/arp.h"
#include "../include/utils.h"
#include "../include/netdev.h"

#define ARP_CACHE_SIZE 256
const uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t request_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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

void arp_reply(struct sk_buff *skb, struct netdev *dev) {
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


    struct eth_hdr *ethhdr = (struct eth_hdr *)skb->data;
    memcpy(ethhdr->dst_mac, arpdata->dmac, 6);
    memcpy(ethhdr->src_mac, arpdata->smac, 6);
    ethhdr->ethertype = ARP_ETHERTYPE;
    ethhdr_dbg("out ", ethhdr);
    ethhdr->ethertype = htons(ethhdr->ethertype);

    ret = netdev_transmit(skb, dev);
}

void arp_recv(struct sk_buff *skb, int len) {
    struct netdev *net_dev;
    int merge = 0;
    struct arp_hdr *arphdr = (struct arp_hdr*)(skb->data + ETH_HDR_LEN);
    arphdr->hwtype = ntohs(arphdr->hwtype);
    arphdr->protype = ntohs(arphdr->protype);
    arphdr->opcode = ntohs(arphdr->opcode);

    arphdr_dbg("in", arphdr);

    if (arphdr->hwtype != ARP_ETHERNET) {
        print_err("ARP: Unsupported HW type\n");
        return;
    }
    
    if (arphdr->protype != ARP_IPV4) {
        print_err("ARP: Unsupported protocol\n");
        return;
    }

    struct arp_ipv4 *arpdata = (struct arp_ipv4*)(skb->data + ETH_HDR_LEN + ARP_HDR_LEN);

    arpdata->dest_ip = ntohl(arpdata->dest_ip);
    arpdata->src_ip = ntohl(arpdata->src_ip);
    arpdata_dbg("in", arpdata);

    merge = arp_cache_update(arphdr, arpdata);
    if (!(net_dev = netdev_get(arpdata->dest_ip))) {
        print_err("ARP was not for us.\n");
        return;
    }

    if ((!merge && arp_cache_insert(arphdr, arpdata)) != 0) {
        print_err("ERR: No free space in ARP translation table\n");
        return;
    }

    if (arphdr->opcode == ARP_REQUEST) {
        arp_reply(skb, net_dev);
    } else if (arphdr->opcode == ARP_REPLY) {
        printf("🎉 Got replied to ARP request.\n");
    }
}

int arp_request(const uint32_t dip/*in network order*/, struct netdev *dev) {
    struct sk_buff *req_skb = skbuff_alloc(2048);
    skb_reserve(req_skb, ARP_HDR_LEN + ETH_HDR_LEN + ARP_DATA_LEN);

    skb_push(req_skb, ARP_DATA_LEN);
    struct arp_ipv4 *arpdata = (struct arp_ipv4 *)req_skb->data;
    memcpy(arpdata->smac, dev->hwaddr, dev->haddr_len);
    arpdata->src_ip = dev->addr;
    memcpy(arpdata->dmac, request_mac, dev->haddr_len);
    arpdata->dest_ip = ntohl(dip);
    arpdata_dbg("req ", arpdata);
    arpdata->src_ip = htonl(arpdata->src_ip);
    arpdata->dest_ip = htonl(arpdata->dest_ip);

    skb_push(req_skb, ARP_HDR_LEN);
    struct arp_hdr *arphdr = (struct arp_hdr *)req_skb->data;
    arphdr->hwtype = ARP_ETHERNET;
    arphdr->protype = ARP_IPV4;
    arphdr->hwlen = dev->haddr_len;
    arphdr->prolen = ARP_IPV4_LEN;
    arphdr->opcode = ARP_REQUEST;
    arphdr_dbg("req ", arphdr);
    arphdr->hwtype = htons(arphdr->hwtype);
    arphdr->protype = htons(arphdr->protype);
    arphdr->opcode = htons(arphdr->opcode);
    
    skb_push(req_skb, ETH_HDR_LEN);
    struct eth_hdr *ethhdr = (struct eth_hdr *)req_skb->data;
    memcpy(ethhdr->dst_mac, broadcast_mac, dev->haddr_len);
    memcpy(ethhdr->src_mac, dev->hwaddr, dev->haddr_len);
    ethhdr->ethertype = ARP_ETHERTYPE;
    ethhdr_dbg("req ", ethhdr);
    ethhdr->ethertype = htons(ethhdr->ethertype);
    
    int ret = netdev_transmit(req_skb, dev);
    free_skb(req_skb);
    return ret;
}

void arp_cache_init() {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry[i] = calloc(1, sizeof(struct arp_entry));
    }
}

void arp_init() {
    arp_cache_init();
}