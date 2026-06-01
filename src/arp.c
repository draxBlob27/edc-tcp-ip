#include "../include/arp.h"
#include "../include/utils.h"

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

void arp_reply(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata, struct netdev *dev) {
    int ret = 0;
    struct eth_hdr *ethhdr = malloc(ETH_HDR_LEN);
    memcpy(ethhdr->dst_mac, arpdata->smac, 6);
    memcpy(ethhdr->src_mac, dev->hwaddr, 6);
    ethhdr->ethertype = ARP_ETHERTYPE;
    printf("\n");
    ethhdr_dbg("out ", ethhdr);
    ethhdr->ethertype = htons(ethhdr->ethertype);

    struct arp_hdr *rep_arphdr = malloc(ARP_HDR_LEN);
    rep_arphdr->hwtype = arphdr->hwtype;
    rep_arphdr->protype = arphdr->protype;
    rep_arphdr->hwlen = arphdr->hwlen;
    rep_arphdr->prolen = arphdr->prolen;
    rep_arphdr->opcode = ARP_REPLY;
    arphdr_dbg("out ", rep_arphdr);
    rep_arphdr->opcode = htons(rep_arphdr->opcode);
    rep_arphdr->hwtype = htons(arphdr->hwtype);
    rep_arphdr->protype = htons(arphdr->protype);

    struct arp_ipv4 *rep_arpdata = malloc(ARP_DATA_LEN);
    memcpy(rep_arpdata->smac, dev->hwaddr, 6);
    rep_arpdata->src_ip = dev->addr;
    memcpy(rep_arpdata->dmac, arpdata->smac, 6);
    rep_arpdata->dest_ip = arpdata->src_ip;
    arpdata_dbg("out ", rep_arpdata);
    rep_arpdata->dest_ip = htonl(arpdata->src_ip);
    rep_arpdata->src_ip = htonl(dev->addr);

    char *buffer = construct_buffer(ethhdr, rep_arphdr, rep_arpdata);
    ret = tun_write(buffer, ETH_HDR_LEN + ARP_HDR_LEN + ARP_DATA_LEN, dev);
}

void arp_recv(void *buffer, int len) {
    struct netdev *net_dev;
    int merge = 0;
    struct arp_hdr *arphdr = (struct arp_hdr*)(buffer + ETH_HDR_LEN);
    arphdr->hwtype = ntohs(arphdr->hwtype);
    arphdr->protype = ntohs(arphdr->protype);
    arphdr->opcode = ntohs(arphdr->opcode);
    arphdr_dbg("in", arphdr);

    if (arphdr->hwtype != ARP_ETHERNET) {
        printf("ARP: Unsupported HW type\n");
        return;
    }
    
    if (arphdr->protype != ARP_IPV4) {
        printf("ARP: Unsupported protocol\n");
        return;
    }

    struct arp_ipv4 *arpdata = (struct arp_ipv4*)(buffer + ETH_HDR_LEN + ARP_HDR_LEN);

    arpdata->dest_ip = ntohl(arpdata->dest_ip);
    arpdata->src_ip = ntohl(arpdata->src_ip);
    arpdata_dbg("data", arpdata);

    merge = arp_cache_update(arphdr, arpdata);
    if (!(net_dev = netdev_get(arpdata->dest_ip))) {
        printf("ARP was not for us.\n");
        return;
    }

    if ((!merge && arp_cache_insert(arphdr, arpdata)) != 0) {
        perror("ERR: No free space in ARP translation table\n");
        return;
    }

    if (arphdr->opcode == ARP_REQUEST) {
        arp_reply(arphdr, arpdata, net_dev);
    } else if (arphdr->opcode == ARP_REPLY) {
        printf("Got replied to request.\n");
    }
}

void arp_request(const uint32_t dip, struct netdev *dev) {
    struct eth_hdr *ethhdr = malloc(ETH_HDR_LEN);
    struct arp_hdr *arphdr = malloc(ARP_HDR_LEN);
    struct arp_ipv4 *arpdata = malloc(ARP_DATA_LEN);

    memcpy(ethhdr->dst_mac, broadcast_mac, dev->haddr_len);
    memcpy(ethhdr->src_mac, dev->hwaddr, dev->haddr_len);
    ethhdr->ethertype = ARP_ETHERTYPE;
    ethhdr_dbg("req ", ethhdr);
    ethhdr->ethertype = htons(ethhdr->ethertype);

    arphdr->hwtype = ARP_ETHERNET;
    arphdr->protype = ARP_IPV4;
    arphdr->hwlen = dev->haddr_len;
    arphdr->prolen = ARP_IPV4_LEN;
    arphdr->opcode = ARP_REQUEST;
    arphdr_dbg("req ", arphdr);
    arphdr->hwtype = htons(arphdr->hwtype);
    arphdr->protype = htons(arphdr->protype);
    arphdr->opcode = htons(arphdr->opcode);

    memcpy(arpdata->smac, dev->hwaddr, dev->haddr_len);
    arpdata->src_ip = dev->addr;
    memcpy(arpdata->dmac, request_mac, dev->haddr_len);
    arpdata->dest_ip = dip;
    arpdata_dbg("req ", arpdata);
    arpdata->src_ip = htonl(arpdata->src_ip);
    arpdata->dest_ip = htonl(arpdata->dest_ip);

    char *buffer = construct_buffer(ethhdr, arphdr, arpdata);
    int ret = tun_write(buffer, ETH_HDR_LEN + ARP_HDR_LEN + ARP_DATA_LEN, dev);
}

void arp_cache_init() {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_entry[i] = calloc(1, sizeof(struct arp_entry));
    }
}

void arp_init() {
    arp_cache_init();
}