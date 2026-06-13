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
    from the remote TCP, or an acknowledgment of the connection
    termination request previously sent.*/
    FIN_WAIT_2, /*represents waiting for a connection termination request
    from the remote TCP.*/
    CLOSE_WAIT, /*represents waiting for a connection termination request
    from the local user.*/
    CLOSING, /*represents waiting for a connection termination request
    acknowledgment from the remote TCP.*/
    LAST_ACK, /*represents waiting for an acknowledgment of the
    connection termination request previously sent to the remote TCP
    (which includes an acknowledgment of its connection termination
    request).*/
    TIME_WAIT, /*represents waiting for enough time to pass to be sure
    the remote TCP received the acknowledgment of its connection
    termination request.*/
    CLOSED /*represents no connection state at all.*/
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
    uint8_t zeros;
    uint8_t protocol;
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

struct tcp_conn *connections[MAX_CONNECTIONS];
struct tcp_port_info *ports[MAX_CONNECTIONS];

uint16_t tcp_checksum(struct tcp_pseudo_hdr *tcp_pseudohdr, struct sk_buff *skb);
struct tcp_conn *tcp_conn_find(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);
struct tcp_conn *tcp_conn_new(uint32_t src_ip, uint32_t dest_ip, \
    uint32_t src_port, uint32_t dest_port);

static inline struct tcp_hdr *tcp_header(struct sk_buff *skb) {
    return (struct tcp_hdr *)(skb->data + ETH_HDR_LEN + IPV4_HDR_LEN);
}

uint32_t tcp_listen(uint16_t port_number);
void tcp_recv();
void tcp_syn_ack(struct tcp_conn *conn);
void tcp_process();
void tcp_send_segment();
void generate_isn();

#endif //TCP_H