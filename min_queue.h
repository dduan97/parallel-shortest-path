#ifndef __MIN_QUEUE_H__
#define __MIN_QUEUE_H__

#include <stdlib.h>
#include "helpers.h"

typedef struct {
    int key;
    WEIGHT val;
    int idx;
} MQNode;

// this will be a key-value store.
typedef struct {
    int capacity;
    int n_items;
    MQNode **arr;
} MinQueue;

MinQueue *mqueue_init(int capacity);

// returns 0 on success, -1 on fail
int mqueue_insert(MinQueue *mq, MQNode *mqn);

MQNode *mqueue_pop_min(MinQueue *mq);
MQNode *mqueue_peek_min(MinQueue *mq);

int mqueue_is_empty(MinQueue *mq);

void mqueue_update_val(MinQueue *mq, MQNode *mqn, WEIGHT new_val);

void mqueue_free(MinQueue *mq, int dynamic_nodes);

void mqueue_print(MinQueue *mq);

#endif
