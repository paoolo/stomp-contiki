
#ifndef STOMP_QUEUE_H
#define	STOMP_QUEUE_H

struct stomp_queue_node {
    struct stomp_queue_node *next;
    char *buf;
    int len;
};

struct stomp_queue {
    struct stomp_queue_node *head, *tail;
};

struct stomp_queue* stomp_queue_new();

void stomp_queue_add(char *buf, int len, struct stomp_queue *queue);

char* stomp_queue_get_buf(struct stomp_queue *queue);

int stomp_queue_get_len(struct stomp_queue *queue);

void stomp_queue_remove(struct stomp_queue *queue);

void stomp_queue_delete(struct stomp_queue *queue);

#endif	/* STOMP_QUEUE_H */

