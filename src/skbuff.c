#include "../include/skbuff.h"

struct sk_buff *skbuff_alloc(size_t size) {
    /*
        initializes socket buffer DS. allocates a buffer and assigns it to the buffer.
        socket buffer is just the metadata holder, actual data is in between the data and tail.
    */
    struct sk_buff *skb = malloc(sizeof(struct sk_buff));
    uint8_t *buffer = malloc(size);
    skb->head = buffer;
    skb->data = buffer;
    skb->tail = buffer;
    skb->end = buffer + size;
    skb->len = 0;
    return skb;
}

/*
    Increases the headroom of an empty skb by reducing the tailroom.
*/
void skb_reserve(struct sk_buff *skb, size_t len) {
    skb->data += len;
}

/*
    Adds data to the start of a buffer; this method decrements the data pointer of the specified skb by the specified len and increments the length of the specified skb by the specified len.
*/
void *skb_push(struct sk_buff *skb, size_t len) {
    skb->data -= len;
    skb->len += len;
    return skb->data;
}

/*
    Removes data from the start of a buffer; this method increments the data pointer of the specified skb by the specified len and decrements the length of the specified skb by the specified len
*/
void *skb_pull(struct sk_buff *skb, size_t len) {
    skb->data += len;
    skb->len -= len;
    return skb->data;
}

/*
    Adds data to a buffer: this method adds len bytes to the buffer of the specified skb and increments the length of the specified skb by the specified len.
*/
void *skb_put(struct sk_buff *skb, size_t len) {
    uint8_t *old_tail = skb->tail;
    skb->tail += len;
    skb->len += len;
    return old_tail;
}

uint8_t *skb_head(struct sk_buff *skb) {
    return skb->head;
}

void free_skb(struct sk_buff *skb) {
    free(skb->head);
    free(skb);
}