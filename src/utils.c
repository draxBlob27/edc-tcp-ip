#include "../include/utils.h"

int tun_write(char *buff, int len, struct netdev *dev) {
    return write(dev->fd, buff, len);
}