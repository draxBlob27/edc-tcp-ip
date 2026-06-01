#include "../include/utils.h"

int tun_write(char *buff, int len, struct netdev *dev) {
    return write(dev->fd, buff, len);
}

char* construct_buffer(struct eth_hdr *ethhdr, struct arp_hdr *arphdr, struct arp_ipv4 *arpdata) {
    char *buffer = malloc(ETH_HDR_LEN + ARP_HDR_LEN + ARP_DATA_LEN);
    memcpy(buffer, ethhdr, ETH_HDR_LEN);
    memcpy(buffer + ETH_HDR_LEN, arphdr, ARP_HDR_LEN);
    memcpy(buffer + ETH_HDR_LEN + ARP_HDR_LEN, arpdata, ARP_DATA_LEN);

    return buffer;
}

void parse_ip(char *addr, uint32_t *dest) {
    if (inet_pton(AF_INET, addr, dest) != 1) {
        perror("ERR: Parsing inet address failed");
        exit(1);
    }
}