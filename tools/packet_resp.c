#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/arp.h"
#include "../include/netdev.h"
#include "../include/utils.h"

int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
    char buffer[2048];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);

    int tuntap_fd = netdev_init(tuntap_name, argv[2], argv[3]);

    uint32_t *dest_ip = calloc(1, ARP_IPV4_LEN);
    uint32_t *src_ip = calloc(1, ARP_IPV4_LEN);
    parse_ip("192.168.0.1", dest_ip);
    parse_ip(argv[2], src_ip);

    *src_ip = ntohl(*src_ip);
    *dest_ip = ntohl(*dest_ip);

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    arp_init();
    while(1) {
        int nread = read(tuntap_fd,buffer,sizeof(buffer));
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }
        
        struct netdev *temp_dev = netdev_get(*src_ip);
        arp_request(*dest_ip, temp_dev);
        parse_ethernet(buffer, nread);
    }
}