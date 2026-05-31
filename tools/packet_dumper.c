#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"

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
    char buffer[2048];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);
    int tuntap_fd = tun_alloc(tuntap_name, IFF_TAP | IFF_NO_PI);  /* tun interface */

    if(tuntap_fd < 0){
      perror("Attaching interface\n");
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