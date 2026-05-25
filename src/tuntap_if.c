#include "../include/tuntap_if.h"

int main() {
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tap0");
    int tun_fd = tun_alloc(tun_name, IFF_TAP | IFF_NO_PI);

    if(tun_fd < 0){
        perror("Allocating interface");
        exit(1);
    }

    printf("Successfully attached/created interface %s\n", tun_name);
    if (ioctl(tun_fd, TUNSETPERSIST, 1) < 0) {
        perror("Enabling TUNSETPERSIST");
        exit(1);
    }

    printf("Successfully persisted interface %s\n", tun_name);

    return 0;
}
