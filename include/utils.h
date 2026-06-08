#ifndef UTILS_H
#define UTILS_H
#include "arp.h"
#include "netdev.h"
#include "ethernet.h"

#define print_dbg(str, ...) \
    printf(str" - %s:%u\n", ##__VA_ARGS__, __FILE__, __LINE__);

#define print_err(str, ...) \
    fprintf(stderr, str, ##__VA_ARGS__);

int tun_write(char *buff, int len, int fd);
void parse_ip(char *addr, uint32_t *dest);

#endif