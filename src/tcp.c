#include "../include/tcp.h"
#include "../include/ipv4.h"
#include "../include/ethernet.h"
#include <time.h>

uint16_t tcp_checksum(struct tcp_pseudo_hdr *tcp_pseudohdr, struct sk_buff *skb) {
    //assumer underlying data is in network order
    //FOR NOW
    uint64_t sm = 0;
    sm += tcp_pseudohdr->src_ip;
    sm += tcp_pseudohdr->dest_ip;
    sm += tcp_pseudohdr->zeros;
    sm += tcp_pseudohdr->protocol;
    sm += tcp_pseudohdr->tcp_length;

    return internet_checksum(skb->data + ETH_HDR_LEN + IPV4_HDR_LEN, skb->len, sm);
}

uint32_t generate_isn() {
    return (uint32_t)time(NULL) * (uint32_t)rand();
}

struct tcp_conn *tcp_conn_find(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_conn *conn = connections[i];
        if (conn->dest_port == dest_port && conn->src_port == src_port \
            && conn->src_ip == src_ip && conn->dest_ip == dest_ip) {
                return conn;
            }

        return NULL;
    }
}

struct tcp_conn *tcp_conn_new(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_conn *conn = connections[i];
        if (!conn->valid) {
            return conn;
        }
    }

    return NULL;
}

