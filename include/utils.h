#ifndef UTILS_H
#define UTILS_H
#include "netdev.h"

int tun_write(char *buff, int len, struct netdev *dev);

#endif