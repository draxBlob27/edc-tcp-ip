#include "../include/tuntap_alloc.h"
#include "../include/ethernet.h"
#include "../include/arp.h"
#include "../include/netdev.h"

int main(int argc, char* argv[]) {
    char tuntap_name[IFNAMSIZ];
    char buffer[2048];
  
    /* Connect to the device */
    strcpy(tuntap_name, argv[1]);

    int tuntap_fd = netdev_init(tuntap_name, argv[2], argv[3]);

    printf("Successfully attached to %s. Waiting for data...\n", tuntap_name);
    while(1) {
        int nread = read(tuntap_fd,buffer,sizeof(buffer));
        if(nread < 0) {
            perror("Reading from interface");
            close(tuntap_fd);
            exit(1);
        }

        parse_ethernet(buffer, nread);
        arp_recv(buffer, nread);
    }
}