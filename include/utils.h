#ifndef UTILS_H
#define UTILS_H
#include "netdev.h"

#define print_dbg(str, ...) \
    printf(str" - %s:%u\n", ##__VA_ARGS__, __FILE__, __LINE__);

#define print_err(str, ...) \
    fprintf(stderr, str, ##__VA_ARGS__);

int tun_write(char *buff, int len, struct netdev *dev);

#endif