#ifndef IPV4_H
#define IPV4_H
#define DEBUG_IPV4

#include "syshead.h"
#include "skbuff.h"
#include "netdev.h"

#define IPV4_HDR_LEN sizeof(struct ipv4_hdr)
#define IPV4 0x04

#ifdef DEBUG_IPV4
#define ipv4hdr_dbg(msg, hdr) \
    do {\
    print_dbg("ipv4 "msg" (version: %d, ihl: %d, tos: %d "\
                "len: %d, id: %d, flags: %d, frag_offset: %d, ttl: %d "\
                "protocol: %d, hdr_csum: %d, src_addr: %d.%d.%d.%d "\
                "dest_addr: %d.%d.%d.%d)\n", hdr->version, hdr->ihl, hdr->tos,\
                hdr->len, hdr->id, hdr->flags, hdr->frag_offset, hdr->ttl,\
                hdr->protocol, hdr->hdr_csum, (hdr->src_addr >> 24) & 0xff, (hdr->src_addr >> 16) & 0xff,\
                (hdr->src_addr >> 8) & 0xff, (hdr->src_addr >> 0) & 0xff, (hdr->dest_addr >> 24) & 0xff,\
                (hdr->dest_addr >> 16) & 0xff, (hdr->dest_addr >> 8) & 0xff, (hdr->dest_addr >> 0) & 0xff);\
    } while(0)
#else 
#define ipv4hdr_dbg(msg, hdr)
#endif

struct ipv4_hdr {
    uint8_t ihl : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t frag_offset : 13;
    uint8_t flags : 3;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t hdr_csum;
    uint32_t src_addr;
    uint32_t dest_addr;
    uint8_t data[];
} __attribute__((packed));

uint16_t internet_checksum(void *addr, size_t count);
void ipv4_recv(struct sk_buff *skb, size_t len);
int ipv4_reply(uint32_t dip, uint8_t protocol, struct sk_buff *skb, size_t len, struct netdev *dev);

#endif //IPV4_H