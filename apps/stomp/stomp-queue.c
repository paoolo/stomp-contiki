#include "stomp-queue.h"
#include "stomp-tools.h"

#include "uip-debug.h"

#include <stdlib.h>

struct stomp_queue*
stomp_queue_new() {
    struct stomp_queue *queue = NULL;

    queue = NEW(struct stomp_queue);
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void
stomp_queue_add(char *buf, int len, struct stomp_queue *queue) {
    struct stomp_queue_node *node;

    if (queue != NULL && buf != NULL) {
        node = NEW(struct stomp_queue_node);
        node->buf = buf;
        node->len = len;
        if (queue->tail != NULL) {
            queue->tail->next = node;
        }
        queue->tail = node;
        if (queue->head == NULL) {
            queue->head = node;
        }
    }
}

char*
stomp_queue_get_buf(struct stomp_queue *queue) {
    if (queue != NULL && queue->head != NULL) {
        return queue->head->buf;
    }
    return NULL;
}

int
stomp_queue_get_len(struct stomp_queue *queue) {
    if (queue != NULL && queue->head != NULL) {
        return queue->head->len;
    }
    return -1;
}

void
stomp_queue_remove(struct stomp_queue *queue) {
    struct stomp_queue_node *node;
    if (queue != NULL && queue->head != NULL) {
        node = queue->head;
        queue->head = queue->head->next;
        DELETE(node);
    }
}

void
stomp_queue_delete(struct stomp_queue *queue) {
    struct stomp_queue_node *node, *temp;
    if (queue == NULL) {
        return;
    }
    node = queue->head;
    DELETE(queue);
    while (node != NULL) {
        temp = node;
        node = node->next;
        DELETE(temp);
    }
}