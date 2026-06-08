#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/netdev.h"
#include "../include/arp.h"
#include "../include/ipv4.h"
#include "../include/icmpv4.h"
#include "../include/utils.h"
/*
    replies to a ping, if sender mac address not availbel request and tells upper layer to try again.
*/
int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);

    int tuntap_fd = netdev_init(tuntap_name, argv[2], argv[3]);

    uint32_t *dest_ip = calloc(1, ARP_IPV4_LEN);
    uint32_t *src_ip = calloc(1, ARP_IPV4_LEN);
    parse_ip("192.168.0.1", dest_ip);
    parse_ip(argv[2], src_ip);

    *src_ip = ntohl(*src_ip);
    struct netdev *dev = netdev_get(*src_ip);
    // *dest_ip = ntohl(*dest_ip);

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    arp_init();

    

    while(1) {
        struct sk_buff *skb = skbuff_alloc(2048);
        int nread = read(tuntap_fd, skb->data, 2048);
        skb->tail += nread;
        skb->len = nread;
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }
        
        icmpv4_request(*dest_ip, dev);
        
        struct eth_hdr *ethhdr = (struct eth_hdr *)(skb->data);
        ethhdr->ethertype = ntohs(ethhdr->ethertype);

        ethhdr_dbg("in ", ethhdr);

        if (ethhdr->ethertype == ARP_ETHERTYPE) {//ARPING
            arp_recv(skb, nread);
        } else if (ethhdr->ethertype == ARP_IPV4) {//IPv4
            ipv4_recv(skb, nread);
        } else {
            printf("Ipv6 or corrupted\n");
        }

        free_skb(skb);
    }
}