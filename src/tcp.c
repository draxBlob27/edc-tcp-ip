#include <time.h>
#include "../include/tcp.h"
#include "../include/netdev.h"
#include "../include/ipv4.h"

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
            conn->src_ip = dest_ip;
            conn->dest_ip = src_ip;
            conn->src_port= dest_port;
            conn->dest_port = src_port;
            conn->state = SYN_RECEIVED;
            return conn;
        }
    }

    return NULL;
}

int32_t tcp_listen(uint16_t port_number) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_port_info *port = ports[i];
        if (!port->valid) {
            port->valid = 1;
            port->port_no = port_number;
            return 0;
        }
    }

    return -1;
}

int32_t tcp_src_port_id(uint16_t port_number) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_port_info *port = ports[i];
        if (port->valid && port->port_no == port_number) {
            return i;
        }
    }

    return -1;
}

void tcp_syn_ack(struct tcp_conn *conn) {
    struct sk_buff *req_skb = skbuff_alloc(2048);
    // skb_reserve(skb, ETH_HDR_LEN + IPV4_HDR_LEN + TCP_HDR_LEN);

    // skb_push(req_skb, TCP_HDR_LEN);
    struct tcp_hdr *tcphdr = tcp_header(skb);
    tcphdr->src_port = conn->src_port;
    tcphdr->dest_port = conn->dest_port;
    tcphdr->seq_no = conn->snd_una;
    tcphdr->ack_no = conn->rcv_nxt;
    tcphdr->ctl_bits = SYN | ACK;

    /*Checksum pending*/

    struct netdev *dev = netdev_get(conn->src_ip);

    ipv4_reply(conn->dest_ip, IPV4_TCP, skb, TCP_HDR_LEN, dev);
}

void tcp_recv(struct sk_buff *skb) {
    struct tcp_hdr *tcphdr = tcp_header(skb);
    uint32_t src_port = tcphdr->src_port, \
            dest_port = tcphdr->dest_port;

    struct ipv4_hdr *ipv4hdr = ipv4_header(skb);
    uint32_t src_ip = ipv4hdr->src_addr, \
            dest_ip = ipv4hdr->dest_addr;

    if (tcphdr->ctl_bits & SYN) {//SYN message recievied
        //check if it is a pure SYN.
        if (tcphdr->ctl_bits != SYN) {//not a pure SYN
            //sends an RST.
            printf("Invalid msg.\n");
        }

        //check if dest_port is in LISTENING STATE
        int32_t port_id = tcp_src_port_id(dest_port);
        
        if (port_id == -1 || ports[port_id]->state != LISTEN) {
            //will need to send RST.
            printf("Port unavailable.\n");
            return;
        }

        //create a TCB with SYN_RECEVIVED state
        struct tcp_conn *conn = tcp_conn_new(src_ip, dest_ip, src_port, dest_port);
        conn->rcv_nxt = tcphdr->seq_no + 1;
        uint32_t isn = generate_isn();
        conn->snd_una = isn;
        conn->snd_nxt = isn + 1;
        conn->rcv_wnd = tcphdr->window;
        conn->snd_wnd = SENDER_WINDOW_LEN;
        
        //delegate the task of constructing a reply to SYN;
        tcp_syn_ack(conn);
    }
}
