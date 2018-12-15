#include "stdio.h"

#include "min_queue.h"

int main(int argc, char **argv) {
    printf("Testing min_queue\n");

    printf("initializing queue with capacity 10\n");
    MinQueue *mq = mqueue_init(10);

    printf("Inserting (3,3)\n");
    MQNode mqn3 = {3, 3};
    mqueue_insert(mq, &mqn3);

    printf("Inserting (2,2)\n");
    MQNode mqn2 = {2, 2};
    mqueue_insert(mq, &mqn2);

    printf("Inserting (4,4)\n");
    MQNode mqn4 = {4, 4};
    mqueue_insert(mq, &mqn4);

    printf("Inserting (7,7)\n");
    MQNode mqn7 = {7, 7};
    mqueue_insert(mq, &mqn7);

    printf("Inserting (5,5)\n");
    MQNode mqn5 = {5, 5};
    mqueue_insert(mq, &mqn5);

    printf("Inserting (6,6)\n");
    MQNode mqn6 = {6, 6};
    mqueue_insert(mq, &mqn6);

    printf("popping min\n");
    MQNode *min = mqueue_pop_min(mq);
    printf("Got (%d %d)\n", min->key, min->val);

    printf("popping min\n");
    min = mqueue_pop_min(mq);
    printf("Got (%d %d)\n", min->key, min->val);

    printf("popping min\n");
    min = mqueue_pop_min(mq);
    printf("Got (%d %d)\n", min->key, min->val);

    printf("Inserting (1,1)\n");
    MQNode mqn1 = {1, 1};
    mqueue_insert(mq, &mqn1);

    printf("popping min\n");
    min = mqueue_pop_min(mq);
    printf("Got (%d %d)\n", min->key, min->val);



}
