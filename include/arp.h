#ifndef ARP_H
#define ARP_H
#define DEBUG_ARP

#include "syshead.h"
#include "ethernet.h"
#include "time.h"
#include "netdev.h"

#define ETH_HDR_LEN sizeof(struct eth_hdr)
#define ARP_ETHERNET    0x0001
#define ARP_ETHERTYPE   0x0806

#ifdef DEBUG_ARP
#define arphdr_dbg(str, hdr)  \
    do { \
        print_dbg("arp hdr "str": (hwtype: %d, protype: %.04x, " \
                    "hwlen: %d, prolen: %d, opcode: %.04x", \
                    hdr->hwtype, hdr->protype, hdr->hwlen, \
                    hdr->prolen, hdr->opcode); \
    } while(0)


#define arpdata_dbg(str, data)  \
    do { \
        print_dbg("arp data "str": (smac: %02x:%02x:%02x:%02x:%02x:%02x ," \
                    "src_ip: %d.%d.%d.%d " \
                    "dmac: %02x:%02x:%02x:%02x:%02x:%02x , " \
                    "dest_ip: %d.%d.%d.%d ", \
                    data->smac[0], data->smac[1], data->smac[2], \
                    data->smac[3], data->smac[4], data->smac[5], \
                    (data->src_ip >> 24) & 0xff, (data->src_ip >> 16) & 0xff, (data->src_ip >> 8) & 0xff, \
                    (data->src_ip >> 0) & 0xff, data->dmac[0], data->dmac[1], \
                    data->dmac[2], data->dmac[3], data->dmac[4], \
                    data->dmac[5], (data->dest_ip >> 24) & 0xff, (data->dest_ip >> 16) & 0xff,\
                    (data->dest_ip >> 8) & 0xff, (data->dest_ip >> 0) & 0xff); \
    } while(0)
#else
#define arphdr_dbg(str, hdr)
#define arpdata_dbg(str, data)
#endif

struct arp_hdr {
    uint16_t hwtype;     // Hardware type (Ethernet = 1)
    uint16_t protype;     // Protocol type (IPv4 = 0x0800)

    uint8_t  hwlen;      // Hardware address length (MAC = 6)
    uint8_t  prolen;      // Protocol address length (IPv4 = 4)

    uint16_t opcode;      // ARP operation
                        // 1 = request
                        // 2 = reply
    uint8_t data[];
} __attribute__((packed));

struct arp_ipv4 {
    uint8_t smac[6];
    uint32_t src_ip;
    uint8_t dmac[6];
    uint32_t dest_ip;
} __attribute__((packed));

struct arp_entry {
    uint16_t hwtype;
    uint32_t ip;
    uint8_t mac[6];
    time_t timestamp;
    int valid;
};

void arp_init();
void arp_cache_init();
void arp_recv(void *buffer, int len);
void arp_reply(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata, struct netdev *dev);
void arp_request(const uint32_t dip, struct netdev *dev);
int arp_cache_update(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata);
int arp_cache_insert(struct arp_hdr *arphdr, struct arp_ipv4 *arpdata);

#endif //ARP_H