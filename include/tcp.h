#ifndef TCP_H
#define TCP_H

#include "syshead.h"
#include "ethernet.h"
#include "ipv4.h"

#define FIN 0x01 //000001
#define SYN 0x02 //000010
#define RST 0x04 //000100
#define PSH 0x08 //001000
#define ACK 0x10 //010000
#define URG 0x20 //100000

#define TCP_SEND_BUF_SIZE 32
#define TCP_RECV_BUF_SIZE 32
#define MAX_CONNECTIONS 64

#define SENDER_WINDOW_LEN 12

#define TCP_HDR_LEN sizeof(struct tcp_hdr)
#define TCP_PSEUDO_HDR_LEN sizeof(struct tcp_pseudo_hdr)

#define TCP_DBG

#ifdef TCP_DBG
#define tcp_hdr_dbg(hdr, str) \
    do { \
        print_dbg("TCP hdr "str": src port: %u, dest port: %u, "\
            "seq no: %u, ack no: %u, data off: %d, ctl_bits: %.02x "\
            "window: %d, checksum: %u, urg ptr: %d, options: %d ",\
            ntohs(hdr->src_port), ntohs(hdr->dest_port), ntohl(hdr->seq_no), htonl(hdr->ack_no),\
            hdr->data_offset, hdr->ctl_bits, ntohs(hdr->window),\
            ntohs(hdr->checksum),\
            hdr->urg_ptr, ntohl(hdr->options));\
    } while(0)
#else
#define tcp_hdr_dbg(hdr, str);
#endif

enum tcp_state { 
    LISTEN, /*represents waiting for a connection request from any remote
    TCP and port.*/
    SYN_SENT, /*represents waiting for a matching connection request
    after having sent a connection request.*/
    SYN_RECEIVED, /*represents waiting for a confirming connection
    request acknowledgment after having both received and sent a
    connection request.*/
    ESTABLISHED, /*represents an open connection, data received can be
    delivered to the user.  The normal state for the data transfer phase
    of the connection.*/
    FIN_WAIT_1, /*represents waiting for a connection termination request
    termination request previously sent.*/
    //from the remote TCP, or an acknowledgment of the connection
    FIN_WAIT_2, /*represents waiting for a connection termination request
    from the remote TCP.*/
    CLOSE_WAIT, /*represents waiting for a connection termination request
    from the local user
    acknowledgment from the remote TCP.*/
    LAST_ACK, /*represents waiting for an acknowledgment of the
    connection termination request previously sent to the remote TCP
    (which includes an acknowledgment of its connection termination
    request).*/
    TIME_WAIT, /*represents waiting for enough time to pass to be sure
    the remote TCP received the acknowledgment of its connection
    termination request.*/
    CLOSED, /*represents no connection state at all.*/
    CLOSING /*represents waiting for a connection termination request*/
};

struct tcp_hdr {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_no;
    uint32_t ack_no;
    uint8_t reserved : 4;
    uint8_t data_offset : 4;
    uint8_t ctl_bits; /*|U|A|P|R|S|F|*/
    uint16_t window;
    uint16_t checksum;
    uint16_t urg_ptr;
    uint32_t options;
    uint8_t data[];
} __attribute__((packed));

struct tcp_pseudo_hdr {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint16_t zeros_and_protocol;
    uint16_t tcp_length;
};

struct tcp_conn {
    int valid;
    uint32_t src_ip, dest_ip;
    uint16_t src_port, dest_port;

    enum tcp_state state;

    uint32_t snd_una; /*oldest unacknowledged sequence number*/
    uint32_t snd_nxt; /*next sequence number to be sent*/
    uint16_t snd_wnd; /*send window*/
    uint32_t rcv_nxt; /*next sequence number expected on an incoming segments, and
        is the left or lower edge of the receive window*/
    uint16_t rcv_wnd; /*receive window*/

    uint8_t send_buffer[TCP_SEND_BUF_SIZE];
    uint8_t recv_buffer[TCP_RECV_BUF_SIZE];

    size_t recv_len; 
    size_t send_len; 
};

struct tcp_port_info {
    int valid;
    uint16_t port_no;
    enum tcp_state state;
};

uint16_t tcp_checksum(struct tcp_pseudo_hdr *tcp_pseudohdr, struct sk_buff *skb);
struct tcp_conn *tcp_conn_find(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);
struct tcp_conn *tcp_conn_new(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);

static inline struct tcp_hdr *tcp_header(struct sk_buff *skb) {
    return (struct tcp_hdr *)(skb->data + ETH_HDR_LEN + IPV4_HDR_LEN);
}

static inline struct tcp_pseudo_hdr *tcp_pseudo_header(\
    uint32_t src_ip/*network order*/,\
    uint32_t dest_ip/*network order*/,\
    size_t len/*host order*/) {
    struct tcp_pseudo_hdr *tcp_pseudohdr = malloc(TCP_PSEUDO_HDR_LEN);
    tcp_pseudohdr->dest_ip = dest_ip;
    tcp_pseudohdr->src_ip = src_ip;
    tcp_pseudohdr->zeros_and_protocol = 0x6000;
    tcp_pseudohdr->tcp_length = htons(len);

    return tcp_pseudohdr;
}

uint16_t tcp_checksum(struct tcp_pseudo_hdr *tcp_pseudohdr, struct sk_buff *skb);
void tcp_recv(uint32_t src_ip, uint32_t dest_ip, struct sk_buff *skb);
struct tcp_conn *tcp_conn_find(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);
struct tcp_conn *tcp_conn_new(uint32_t  src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);
int32_t tcp_listen(uint16_t port_number);
int32_t tcp_src_port_id(uint16_t port_number);
void tcp_send_segment(struct tcp_conn *conn, uint8_t ctl_bits, \
    uint8_t *payload, size_t payload_len);
uint32_t generate_isn();
void tcp_init();
void tcp_open_port(uint16_t port_no);

#endif //TCP_H