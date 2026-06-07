#include "../include/utils.h"

int tun_write(char *buff, int len, int fd) {
    return write(fd, buff, len);
}

void parse_ip(char *addr, uint32_t *dest) {
    if (inet_pton(AF_INET, addr, dest) != 1) {
        perror("ERR: Parsing inet address failed");
        exit(1);
    }
}