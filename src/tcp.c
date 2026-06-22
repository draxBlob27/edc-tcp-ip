#include <time.h>
#include "../include/tcp.h"
#include "../include/netdev.h"
#include "../include/ipv4.h"
#include "../include/utils.h"

struct tcp_conn *connections[MAX_CONNECTIONS];
struct tcp_port_info *ports[MAX_CONNECTIONS];

void tcp_init() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i] = calloc(1, sizeof(struct tcp_conn));
        ports[i] = calloc(1, sizeof(struct tcp_port_info));
        ports[i]->state = CLOSED;
    }
}

void tcp_open_port(uint16_t port_no) {
    ports[0]->port_no = 8080;
    ports[0]->state = LISTEN;
    ports[0]->valid = 1;
}

uint16_t tcp_checksum(struct tcp_pseudo_hdr *tcp_pseudohdr, struct sk_buff *skb) {
    //assumer underlying data is in network order
    uint64_t partial_sm = internet_checksum_partial(tcp_pseudohdr, TCP_PSEUDO_HDR_LEN, 0);
    uint64_t csum = internet_checksum_partial(tcp_header(skb), ntohs(tcp_pseudohdr->tcp_length), partial_sm);

    return internet_checksum_final(csum);
}

uint32_t generate_isn() {
    return (uint32_t)time(NULL) * (uint32_t)rand();
}

struct tcp_conn *tcp_conn_find(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_conn *conn = connections[i];
        if (conn->dest_port == src_port && \
            conn->src_port == dest_port && \
            conn->src_ip == dest_ip && conn->dest_ip == src_ip) {
                return conn;
            }

        return NULL;
    }
}

struct tcp_conn *tcp_conn_new(uint32_t src_ip/*network order*/,\
                uint32_t dest_ip/*network order*/, \
                uint32_t src_port/*network order*/, \
                uint32_t dest_port/*network order*/) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_conn *conn = connections[i];
        if (!conn->valid) {
            conn->src_ip = dest_ip;
            conn->dest_ip = src_ip;
            conn->src_port= dest_port;
            conn->dest_port = src_port;
            conn->state = LISTEN;
            conn->valid = 1;
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

int32_t tcp_src_port_id(uint16_t port_number/*host order*/) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_port_info *port = ports[i];
        if (port->valid && port->port_no == port_number) {
            return i;
        }
    }
    return -1;
}

void tcp_send_segment(struct tcp_conn *conn, uint8_t ctl_bits, \
    uint8_t *payload, size_t payload_len) {
    struct sk_buff *req_skb = skbuff_alloc(2048);
    // skb_reserve(skb, ETH_HDR_LEN + IPV4_HDR_LEN + TCP_HDR_LEN);
    // skb_push(req_skb, TCP_HDR_LEN);
    struct tcp_hdr *tcphdr = tcp_header(req_skb);

    tcphdr->src_port = conn->src_port;
    tcphdr->dest_port = conn->dest_port;
    tcphdr->seq_no = conn->snd_una;
    tcphdr->ack_no = conn->rcv_nxt;
    tcphdr->ctl_bits = ctl_bits;
    tcphdr->data_offset = 5;
    tcphdr->checksum = 0;
    tcphdr->window = conn->rcv_wnd;
    memcpy(tcphdr->data, payload, payload_len);

    struct tcp_pseudo_hdr *tcp_psuedohdr = tcp_pseudo_header(conn->src_ip,conn->dest_ip, tcphdr->data_offset * 4 + payload_len);

    tcphdr->checksum = tcp_checksum(tcp_psuedohdr, req_skb);
    struct netdev *dev = netdev_get(ntohl(conn->src_ip));

    tcp_hdr_dbg(tcphdr, " out: ");

    ipv4_reply(conn->dest_ip, IPV4_TCP, req_skb, tcphdr->data_offset * 4 + payload_len, dev);
}

void tcp_recv(uint32_t src_ip/*network order*/, \
    uint32_t dest_ip/*network order*/, \
    struct sk_buff *skb) {
    struct tcp_hdr *tcphdr = tcp_header(skb); //network order;
    tcp_hdr_dbg(tcphdr, "in");

    uint32_t src_port = tcphdr->src_port, \
            dest_port = tcphdr->dest_port;

    struct tcp_pseudo_hdr *tcp_pseudohdr = tcp_pseudo_header(src_ip, dest_ip, \
                                        (size_t)(skb->len - ((uint8_t *)tcphdr - skb->head)));

    uint16_t recv_checksum = tcp_checksum(tcp_pseudohdr, skb);
    if (recv_checksum != 0) {
        print_err("Segment courrpted.\n");
        return;
    }

    //check if dest_port is in LISTENING STATE
    int32_t port_id = tcp_src_port_id(ntohs(dest_port));
    
    if (port_id == -1 || ports[port_id]->state != LISTEN) {
        //will need to send RST.
        printf("Port unavailable.\n");
        struct tcp_conn *conn = tcp_conn_new(src_ip, dest_ip,\
                                        src_port, dest_port);

        conn->snd_una = tcphdr->ack_no;
        conn->rcv_nxt = htonl(ntohl(tcphdr->seq_no) + 1);
        tcp_send_segment(conn, RST | ACK, NULL, 0);
        // tcp_send_rst();
        conn->valid = 0;
        return;
    }

    //find for TCB
    struct tcp_conn *conn = tcp_conn_find(src_ip, dest_ip, src_port, dest_port);
    if (!conn) {
        //create a TCB with SYN_RECEVIVED state
        conn = tcp_conn_new(src_ip, dest_ip,\
                                        src_port, dest_port);
    }
        

    switch(conn->state) {
        case LISTEN: {
            //transition to SYN_RECV
            if (tcphdr->ctl_bits == SYN) {
                conn->state = SYN_RECEIVED;
                conn->rcv_nxt = htonl(ntohl(tcphdr->seq_no) + 1); //network
                uint32_t isn = generate_isn();
                conn->snd_una = htonl(isn); //network
                conn->rcv_wnd = tcphdr->window; //network
                conn->snd_wnd = SENDER_WINDOW_LEN; //
    
                tcp_send_segment(conn, SYN | ACK, NULL, 0);
            }
            break;
        }
        case SYN_RECEIVED: {
            //trnsion to ESTAB
            if (tcphdr->ctl_bits == ACK) {
                conn->state = ESTABLISHED;
                printf("Connection established\n.");
                conn->rcv_nxt = tcphdr->seq_no; //network
                conn->snd_una = tcphdr->ack_no; //network
                conn->rcv_wnd = tcphdr->window; //network
                conn->snd_wnd = SENDER_WINDOW_LEN; //
                tcp_send_segment(conn, FIN | ACK, NULL, 0);
                conn->state = FIN_WAIT_1;
            }
            break;
        }
        case FIN_WAIT_1: {
            if (tcphdr->ctl_bits == ACK) {
                conn->state = FIN_WAIT_2;
            }
            break;
        }
        case FIN_WAIT_2: {
            if (tcphdr->ctl_bits == (FIN | ACK)) {
                conn->state = TIME_WAIT;
                conn->rcv_nxt = htonl(ntohl(tcphdr->seq_no) + 1); //network
                conn->snd_una = tcphdr->ack_no;
                conn->rcv_wnd = tcphdr->window; //network
                conn->snd_wnd = SENDER_WINDOW_LEN; //
                tcp_send_segment(conn, ACK, NULL, 0);
                conn->state = CLOSED;
            }
            break;
        }
        case CLOSED: {
            conn->valid = 0;
        }

        default: {
            print_err("Not mature enough stack.\n");
            break;
        }
    }
}
