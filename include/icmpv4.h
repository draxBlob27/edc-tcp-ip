#ifndef ICMPV4_H
#define ICMPV4_H
#define ICMPV4_DEBUG

#include "syshead.h"
#include "skbuff.h"
#include "netdev.h"

#define ICMPV4_HDR_LEN sizeof(struct icmpv4_hdr)
#define ICMPV4 0x01
#define ECHO_REQUEST 0x08
#define ECHO_REPLY 0

#ifdef ICMPV4_DEBUG
#define icmpv4hdr_dbg(msg, hdr) \
    do {\
        print_dbg("icmpv4 "msg" (type: %d, code: %d, checksum: %d, indentifier: %d" \
            "sequence_no: %d\n)", hdr->type, hdr->code, hdr->checksum, hdr->identifier,\
            hdr->sequence_no);\
    }\
    while(0)
#else 
#define icmpv4hdr_dbg(msg, hdr)
#endif


struct icmpv4_hdr {
    uint8_t type;
    uint8_t code;  
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_no;
    uint8_t data[];
} __attribute__((packed));

void icmpv4_recv(struct sk_buff *skb, size_t len, struct netdev *dev);
void icmpv4_reply(struct sk_buff *skb, size_t len, struct netdev *dev);
int icmpv4_request(const uint32_t dip, struct netdev *dev);
#endif //ICMPV4_H