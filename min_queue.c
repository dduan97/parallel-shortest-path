#include "min_queue.h"

//helper function to swap two nodes in an array
static void swap(MinQueue *mq, size_t idx1, size_t idx2) {
    MQNode *n1 = mq->arr[idx1];
    mq->arr[idx1] = mq->arr[idx2];
    mq->arr[idx2] = n1;
    mq->arr[idx1]->idx = idx1;
    mq->arr[idx2]->idx = idx2;
}

// helper function to propogate a node up
static size_t up_dog(MinQueue *mq, size_t i) {
    while (i > 1) {
        if (mq->arr[i]->val > mq->arr[i / 2]->val) {
            break;
        }
        // then we swap
        swap(mq, i, i/2);
        i /= 2;
    }
    return i;
}

// helper function to propogate a node up
static size_t downward_dog(MinQueue *mq, size_t i) {
    while (2*i <= mq->n_items) {
        // if there is no right child
        if (2*i == mq->n_items) {
            // smaller than left child, then break
            if (mq->arr[i]->val <= mq->arr[2*i]->val) {
                break;
            }

            swap(mq, i, 2*i);
            i = 2*i;
            continue;
        }

        // otherwise we choose the smaller child
        size_t child_idx = 2*i;
        if (mq->arr[2*i]->val > mq->arr[2*i + 1]->val) {
            child_idx = 2*i + 1;
        }

        // now we swap with child_idx
        swap(mq, i, child_idx);
        i = child_idx;
    }
    return i;
}

MinQueue *mqueue_init(size_t capacity) {
    MinQueue *mq = malloc(sizeof(MinQueue));
    mq->capacity = capacity;
    mq->arr = malloc(capacity * sizeof(MQNode));
    mq->n_items = 0;
    return mq;
}

size_t mqueue_insert(MinQueue *mq, MQNode *mqn) {
    if (mq->n_items == mq->capacity) {
       return -1;
    }

    // otherwise, we insert into the end and bubble up
    // since the root node should be at index 1, increment n_items before assigning
    mq->arr[++mq->n_items] = mqn;
    up_dog(mq, mq->n_items);

    return 0;
}

MQNode *mqueue_pop_min(MinQueue *mq) {
    if (mqueue_is_empty(mq)) {
        return NULL;
    }
    MQNode *min = mq->arr[1];
    mq->arr[1] = mq->arr[mq->n_items];
    mq->arr[1]->idx = 1;
    mq->arr[mq->n_items] = NULL;
    mq->n_items--;
    downward_dog(mq, 1);
    return min;
}

int mqueue_is_empty(MinQueue *mq) {
    return (mq->n_items == 0);
}

void mqueue_update_val(MinQueue *mq, MQNode *mqn, WEIGHT new_val) {
    size_t i = mqn->idx;
    WEIGHT old_val = mqn->val;
    mqn->val = new_val;
    // increasing priority means moving down in the heap
    if (new_val > old_val) {
        i = downward_dog(mq, i);
    } else if (new_val < old_val) {
        up_dog(mq, i);
    }
}

void mqueue_free(MinQueue *mq, int dynamic_nodes) {
    if (dynamic_nodes) {
        for (size_t i = 0; i < mq->n_items; i++) {
            free(mq->arr[i]);
        }
    }
    free(mq->arr);
    free(mq);
}

void mqueue_print(MinQueue *mq) {
    for (size_t i = 1; i <= mq->n_items; i++) {
        MQNode *mqn = mq->arr[i];
        printf("(%zd %d), ", mqn->key, mqn->val);
    }
    printf("\n");
}

