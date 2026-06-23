#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/skbuff.h"

/*
    Dumps ethernet raw bytes as is on to the screen.
*/

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

int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);
    int tuntap_fd = tun_alloc(tuntap_name, IFF_TAP | IFF_NO_PI);  /* tun interface */

    if(tuntap_fd < 0){
      perror("Attaching interface\n");
      exit(1);
    }

    struct sk_buff *skb = skbuff_alloc(2048);

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    while(1) {
        
        int nread = read(tuntap_fd,skb->data, 2048);
        skb->tail += nread;
        skb->len = nread;
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }

        struct eth_hdr *ethhdr = (struct eth_hdr *)skb->data;

        struct arp_hdr *arphdr = (struct arp_hdr *)(skb->data + ETH_HDR_LEN);
        ethhdr->ethertype = ntohs(ethhdr->ethertype);

        ethhdr_dbg("in ", ethhdr);

        if (ethhdr->ethertype == ARP_ETHERTYPE) {//ARPING
            printf("ARP message\n");
        } else if (ethhdr->ethertype == 0x0800) {//IPv4
            printf("Ipv4 address\n");
        } else {
            printf("Ipv6 or corrupted\n");
        }

        free_skb(skb);
    }
}