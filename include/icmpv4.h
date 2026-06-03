#ifndef ICMPV4_H
#define ICMPV4_H

#include "syshead.h"

#define ICMPV4_HDR_LEN sizeof(struct icmpv4_hdr)
#define ICMPV4 0x01

struct icmpv4_hdr {
    uint8_t type;
    uint8_t code;  
    uint16_t checksum;
    uint32_t unused;
} __attribute__((packed));

void icmpv4_recv(struct ipv4_hdr *ipv4, char *payload, size_t len);
#endif //ICMPV4_H