#include "min_queue.h"

//helper function to swap two nodes in an array
static void swap(MinQueue *mq, int idx1, int idx2) {
    MQNode *n1 = mq->arr[idx1];
    mq->arr[idx1] = mq->arr[idx2];
    mq->arr[idx2] = n1;
    mq->arr[idx1]->idx = idx1;
    mq->arr[idx2]->idx = idx2;
}

// helper function to propogate a node up
static void up_dog(MinQueue *mq, int i) {
    while (i > 1) {
        if (mq->arr[i]->val > mq->arr[i / 2]->val) {
            break;
        }
        // then we swap
        swap(mq, i, i/2);
        i /= 2;
    }
}

// helper function to propogate a node up
static void downward_dog(MinQueue *mq, int i) {
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
        int child_idx = 2*i;
        if (mq->arr[2*i]->val > mq->arr[2*i + 1]->val) {
            child_idx = 2*i + 1;
        }

        // now we swap with child_idx
        swap(mq, i, child_idx);
        i = child_idx;
    }
}

MinQueue *mqueue_init(int capacity) {
    MinQueue *mq = malloc(sizeof(MinQueue));
    mq->capacity = capacity;
    mq->arr = malloc(capacity * sizeof(MQNode));
    mq->n_items = 0;
    return mq;
}

int mqueue_insert(MinQueue *mq, MQNode *mqn) {
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

void mqueue_update_val(MinQueue *mq, MQNode *mqn, int new_val) {
    int i = mqn->idx;
    // increasing priority means moving down in the heap
    if (new_val > mqn->val) {
        downward_dog(mq, i);
    } else if (new_val < mqn->val) {
        up_dog(mq, i);
    }
}

void mqueue_free(MinQueue *mq, int dynamic_nodes) {
    if (dynamic_nodes) {
        for (int i = 0; i < mq->n_items; i++) {
            free(mq->arr[i]);
        }
    }
    free(mq->arr);
    free(mq);
}


