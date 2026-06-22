#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/tcp.h"
#include "../include/arp.h"
/*
    replies to a ping, if sender mac address not availbel request and tells upper layer to try again.
*/
int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);

    int tuntap_fd = netdev_init(tuntap_name, argv[2], argv[3]);

    struct sk_buff *skb = skbuff_alloc(2048);

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);

    tcp_init();
    tcp_open_port(8080);
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
        } else if (ethhdr->ethertype == ARP_IPV4) {//IPv4
            ipv4_recv(skb, nread);
        } else {
            printf("Ipv6 or corrupted\n");
        }
    }
}