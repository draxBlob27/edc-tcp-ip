#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/netdev.h"
#include "../include/arp.h"

/*
    Parses ehternet frames and if message is arp_request, replies it with self mac addr, and adds
    sender mac addr into its cache.
*/
int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);

    int tuntap_fd = netdev_init(tuntap_name, argv[2], argv[3]);

    // uint32_t *dest_ip = calloc(1, ARP_IPV4_LEN);
    // uint32_t *src_ip = calloc(1, ARP_IPV4_LEN);
    // parse_ip("192.168.0.1", dest_ip);
    // parse_ip(argv[2], src_ip);

    // *src_ip = ntohl(*src_ip);
    // *dest_ip = ntohl(*dest_ip);
    struct sk_buff *skb = skbuff_alloc(2048);
    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    arp_init();
    while(1) {
        int nread = read(tuntap_fd, skb->data, 2048);
        skb->tail += nread;
        skb->len = nread;
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }
        
        // struct netdev *temp_dev = netdev_get(*src_ip);
        // arp_request(*dest_ip, temp_dev);
        struct eth_hdr *ethhdr = (struct eth_hdr *)(skb->data);
        ethhdr->ethertype = ntohs(ethhdr->ethertype);

        ethhdr_dbg("in ", ethhdr);

        if (ethhdr->ethertype == ARP_ETHERTYPE) {//ARPING
            arp_recv(skb, nread);
        } else if (ethhdr->ethertype == 0x0800) {//IPv4
            printf("Ipv4 address\n");
        } else {
            printf("Ipv6 or corrupted\n");
        }
    }
}