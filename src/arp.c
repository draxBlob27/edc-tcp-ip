#include "../include/arp.h"
#include "../include/netdev.h"
#include "../include/utils.h"

#define ARP_CACHE_SIZE 256
const uint8_t LMAC[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};

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
        if (!arp_entry[i]) {
            arp_entry[i]->hwtype = arphdr->hwtype;
            arp_entry[i]->ip = arpdata->src_ip;
            memcpy(arp_entry[i]->mac, arpdata->smac, 6);
            arp_entry[i]->valid = 1;
            arp_entry[i]->timestamp = time(NULL);

            return 1;
        }
    }

    return 0;
}

void arp_reply(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata, struct netdev *dev) {
    int ret = 0;
    struct eth_hdr *ethhdr = malloc(ETH_HDR_LEN);
    memcpy(ethhdr->dst_mac, arpdata->smac, 6);
    memcpy(ethhdr->src_mac, dev->hwaddr, 6);
    ethhdr->ethertype = ARP_ETHERTYPE; 

    struct arp_hdr *rep_arphdr = malloc(ARP_HDR_LEN);
    rep_arphdr->hwtype = arphdr->hwtype;
    rep_arphdr->protype = arphdr->protype;
    rep_arphdr->hwlen = arphdr->hwlen;
    rep_arphdr->prolen = arphdr->prolen;
    rep_arphdr->opcode = ARP_REPLY;

    struct arp_ipv4 *rep_arpdata = malloc(ARP_DATA_LEN);
    memcpy(rep_arpdata->smac, dev->hwaddr, 6);
    rep_arpdata->src_ip = htonl(dev->addr);
    memcpy(rep_arpdata->dmac, arpdata->smac, 6);
    rep_arpdata->dest_ip = arpdata->src_ip;

    char *buffer = malloc(ETH_HDR_LEN + ARP_HDR_LEN + ARP_DATA_LEN);
    memcpy(buffer, ethhdr, ETH_HDR_LEN);
    memcpy(buffer + ETH_HDR_LEN, rep_arphdr, ARP_HDR_LEN);
    memcpy(buffer + ETH_HDR_LEN + ARP_HDR_LEN, rep_arpdata, ARP_DATA_LEN);

    ret = tun_write(buffer, ETH_HDR_LEN + ARP_HDR_LEN + ARP_DATA_LEN, dev);
}

void arp_recv(void *buffer, int len) {
    struct netdev *net_dev;

    int merge = 0;
    uint8_t *ptr = (uint8_t *)buffer;

    ptr += ETH_HDR_LEN;
    struct arp_hdr *arphdr = (struct arp_hdr*)ptr;
    arphdr->hwtype = ntohs(arphdr->hwtype);
    arphdr->hwtype = ntohs(arphdr->hwtype);
    arphdr->hwtype = ntohs(arphdr->hwtype);

    if (arphdr->hwtype != ARP_ETHERNET) {
        printf("ARP: Unsupported HW type\n");
    }
    
    if (arphdr->protype != ARP_IPV4) {
        printf("ARP: Unsupported protocol\n");
    }

    ptr += ARP_HDR_LEN;
    struct arp_ipv4 *arpdata = (struct arp_ipv4*)ptr;

    arpdata->dest_ip = ntohl(arpdata->dest_ip);
    arpdata->src_ip = ntohl(arpdata->src_ip);

    merge = arp_cache_update(arphdr, arpdata);
    if (!(net_dev = netdev_get(arpdata->dest_ip))) {
        printf("ARP was not for us.\n");
    }

    if ((!merge && arp_cache_insert(arphdr, arpdata)) == 0) {
        perror("ERR: No free space in ARP translation table\n");
    }

    if (arphdr->opcode == ARP_REQUEST) {
        arp_reply(arphdr, arpdata, net_dev);
    } else {
        printf("ARP: Opcode not supported\n");
    }
}


