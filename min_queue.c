#include "min_queue.h"

//helper function to swap two nodes in an array
static void swap(MinQueue *mq, int idx1, int idx2) {
    MQNode *n1 = mq->arr[idx1];
    mq->arr[idx1] = mq->arr[idx2];
    mq->arr[idx2] = n1;
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
    int idx = mq->n_items;
    while (idx > 1) {
        if (mq->arr[idx]->val > mq->arr[idx / 2]->val) {
            break;
        }
        // then we swap
        swap(mq, idx, idx/2);
        idx /= 2;
    }

    return 0;
}

MQNode *mqueue_pop_min(MinQueue *mq) {
    MQNode *min = mq->arr[1];
    mq->arr[1] = mq->arr[mq->n_items];
    mq->arr[mq->n_items] = NULL;
    mq->n_items--;
    int i = 1;
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
    return min;
}

void mqueue_free(MinQueue *mq) {
    free(mq->arr);
    free(mq);
}
