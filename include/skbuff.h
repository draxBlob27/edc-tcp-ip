#ifndef SKBUFF_H
#define SKBUFF_H

#include "syshead.h"

struct sk_buff {
    uint8_t *head;
    uint8_t *data;
    uint8_t *tail;
    uint8_t *end;
    uint32_t len;
};

struct sk_buff *skbuff_alloc(size_t size);
void skb_reserve(struct sk_buff *skb, size_t len);
void *skb_push(struct sk_buff *skb, size_t len);
void *skb_pull(struct sk_buff *skb, size_t len);
void *skb_put(struct sk_buff *skb, size_t len);
uint8_t *skb_head(struct sk_buff *skb);
void free_skb(struct sk_buff *skb);

#endif //SKBUFF_H
