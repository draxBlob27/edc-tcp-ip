#ifndef IPV4_H
#define IPV4_H
#define DEBUG_IPV4

#include "syshead.h"

#define IPV4_HDR_LEN sizeof(struct ipv4_hdr)
#define IPV4 0x04

#ifdef DEBUG_IPV4
#define ipv4hdr_dbg(msg, hdr) \
    do {\
    print_dbg("ipv4 "msg" (version: %d, ihl: %d, tos: %d "\
                "len: %d, id: %d, flags: %d, frag_offset: %d, ttl: %d "\
                "protocol: %d, hdr_csum: %d, src_addr: %d.%d.%d.%d "\
                "dest_addr: %d.%d.%d.%d", hdr->version, hdr->ihl, hdr->tos,\
                hdr->len, hdr->id, hdr->flags, hdr->frag_offset, hdr->ttl,\
                hdr->protocol, hdr->hdr_dcsum, (data->src_ip >> 24) & 0xff, (data->src_ip >> 16) & 0xff,\(data->src_ip >> 8) & 0xff, (data->src_ip >> 0) & 0xff, (data->dest_ip >> 24) & 0xff,\(data->dest_ip >> 16) & 0xff, (data->dest_ip >> 8) & 0xff, (data->dest_ip >> 0) & 0xff)\
    } while(0)
#else 
#define ipv4hdr_dbg(msg, hdr)
#endif

struct ipv4_hdr {
    uint8_t version : 4;
    uint8_t ihl : 4;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint8_t flags : 3;
    uint16_t frag_offset : 13;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t hdr_csum;
    uint32_t src_addr;
    uint32_t dest_addr;
    uint8_t data[];
} __attribute__((packed));

uint16_t ipv4_checksum(void *addr, size_t count);
void ipv4_recv(char *buffer, size_t len);
void ipv4_send(uint32_t dip, uint8_t protocol, char *payload, size_t len);

#endif //IPV4_H