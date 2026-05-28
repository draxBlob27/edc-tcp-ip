#include "../include/tuntap_if.h"
#include "../include/ehternet.h"

void parse_ethernet(void *buffer, int len) {
    struct eth_hdr *hdr = (struct eth_hdr *)buffer;
    hdr->ethertype = ntohs(hdr->ethertype);

    printf("dmac: %02x:%02x:%02x:%02x:%02x:%02x "\
        "smac: %02x:%02x:%02x:%02x:%02x:%02x "\
        "ethertype: 0x%04x\n", hdr->dst_mac[0], hdr->dst_mac[1], hdr->dst_mac[2],\
        hdr->dst_mac[3], hdr->dst_mac[4], hdr->dst_mac[5], hdr->src_mac[0],\
        hdr->src_mac[1], hdr->src_mac[2], hdr->src_mac[3], hdr->src_mac[4],\
        hdr->src_mac[5], hdr->ethertype);

    if (hdr->ethertype == 0x0806) {//ARPING
        printf("arping\n")
    } else if (hdr->ethertype == 0x0800) {//IPv4
        print("Ipv4 address\n");
    } else {
        printf("Ipv6 or corrupted\n");
    }
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