#include <stdlib.h>

typedef struct {
    int key;
    int val;
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

void mqueue_free(MinQueue *mq);
