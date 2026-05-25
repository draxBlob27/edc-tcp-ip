#include "../include/tuntap_if.h"
#include <stdint.h>

struct eth_hdr {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
} __attribute__((packed));

void parse_ethernet(void *buffer, int len) {
    struct eth_hdr *hdr = (struct eth_hdr *)buffer;
    hdr->ethertype = ntohs(hdr->ethertype);

    printf("dmac: %02d:%02x:%02x:%02x:%02x:%02x "\
        "smac: %02x:%02x:%02x:%02x:%02x:%02x "\
        "ethertype: %04x\n", hdr->dst_mac[0], hdr->dst_mac[1], hdr->dst_mac[2],\
        hdr->dst_mac[3], hdr->dst_mac[4], hdr->dst_mac[5], hdr->src_mac[0],\
        hdr->src_mac[1], hdr->src_mac[2], hdr->src_mac[3], hdr->src_mac[4],\
        hdr->src_mac[5], hdr->ethertype);
}

int main() {
    char tuntap_name[IFNAMSIZ];
    char buffer[2048];
  
    /* Connect to the device */
    strcpy(tuntap_name, "tap0");
    int tuntap_fd = tun_alloc(tuntap_name, IFF_TAP | IFF_NO_PI);  /* tun interface */

    if(tuntap_fd < 0){
      perror("Allocating interface");
      exit(1);
    }

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    while(1) {
        int nread = read(tuntap_fd,buffer,sizeof(buffer));
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }

        parse_ethernet(buffer, nread);
    }
}