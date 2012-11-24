#include "stomp-global.h"

#include "stomp-queue.h"
#include "stomp-tools.h"

#include "uip-debug.h"

#include <stdlib.h>

struct stomp_queue* stomp_queue_new() {
    struct stomp_queue *queue = NULL;

    queue = NEW(struct stomp_queue);
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void stomp_queue_add(char *buf, int len, struct stomp_queue *queue) {
    struct stomp_queue_node *node;

#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_add: start.\n");
#endif

    if (queue != NULL && buf != NULL) {
        node = NEW(struct stomp_queue_node);
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_add: new node added.\n");
#endif
        node->buf = buf;
        node->len = len;
        if (queue->tail != NULL) {
            queue->tail->next = node;
#ifdef STOMP_QUEUE_TRACE
            PRINTA("stomp_queue_add: there was a tail in the queue, moved.\n");
#endif
        }
        queue->tail = node;
        if (queue->head == NULL) {
            queue->head = node;
#ifdef STOMP_QUEUE_TRACE
            PRINTA("stomp_queue_add: there was nothing on the head.\n");
#endif
        }
    }

#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_trace: stop.\n");
#endif
}

char* stomp_queue_get_buf(struct stomp_queue *queue) {
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_get_buf: start.\n");
#endif
    if (queue != NULL && queue->head != NULL) {
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_get_buf: return: %s.\n", queue->head->buf);
#endif
        return queue->head->buf;
    }
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_get_buf: return: NULL.\n");
#endif
    return NULL;
}

int stomp_queue_get_len(struct stomp_queue *queue) {
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_get_len: start.\n");
#endif
    if (queue != NULL && queue->head != NULL) {
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_get_len: return: %d\n", queue->head->len);
#endif
        return queue->head->len;
    }
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_get_len: return: -1.\n");
#endif
    return -1;
}

void stomp_queue_remove(struct stomp_queue *queue) {
    struct stomp_queue_node *node;
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_remove: start.\n");
#endif
    if (queue != NULL && queue->head != NULL) {
        node = queue->head;
        queue->head = queue->head->next;
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_remove: delete node.\n");
#endif
        DELETE(node);
    }
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_remove: stop.\n");
#endif
}

void stomp_queue_delete(struct stomp_queue *queue) {
    struct stomp_queue_node *node, *temp;
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_delete: start.\n");
#endif
    if (queue == NULL) {
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_delete: nothing to delete, stop.\n");
#endif
        return;
    }

    node = queue->head;
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_delete: deleting queue struct.\n");
#endif
    DELETE(queue);
    while (node != NULL) {
        temp = node;
        node = node->next;
#ifdef STOMP_QUEUE_TRACE
        PRINTA("stomp_queue_delete: deleting node struct.\n");
#endif
        DELETE(temp);
    }
#ifdef STOMP_QUEUE_TRACE
    PRINTA("stomp_queue_delete: stop.\n");
#endif
}