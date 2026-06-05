#include "../include/icmpv4.h"
#include "../include/ipv4.h"
#include "../include/ethernet.h"

void icmpv4_recv(char *buffer, size_t len) {
    struct ipv4_hdr *ipv4hdr = (struct ipv4_hdr *)buffer;
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)ipv4hdr->data;

    uint16_t csum = internet_checksum(icmpv4hdr, ICMPV4_HDR_LEN);
    if (csum != 0) {
        print_err("ICMP: Packet corrupted\n");
        return;
    }

    if (icmpv4hdr->type == ECHO_REQUEST) {
        icmpv4_reply(buffer, len);
    }
}

void icmpv4_reply(char *buffer, int len) {
    struct ipv4_hdr *ipv4hdr = (struct ipv4_hdr *)buffer;
    struct icmpv4_hdr *icmpv4hdr = (struct icmpv4_hdr *)ipv4hdr->data;

    
}