#ifndef ICMPV4_H
#define ICMPV4_H

#include "syshead.h"

#define ICMPV4_HDR_LEN sizeof(struct icmpv4_hdr)
#define ICMPV4 0x01
#define ECHO_REQUEST 0x08
#define ECHO_REPLY 0

struct icmpv4_hdr {
    uint8_t type;
    uint8_t code;  
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_no;
    uint8_t data[];
} __attribute__((packed));

void icmpv4_recv(char *buffer, size_t len);
void icmpv4_reply(char *buffer, int len)
#endif //ICMPV4_H