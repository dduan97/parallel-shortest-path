#ifndef __RESULTR_H__
#define __RESULTR_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "flat_matrix.h"

typedef enum {
    ALGO_SER_DIJKSTRA,
    ALGO_SER_BF,
    ALGO_PAR_DIJKSTRA,
    ALGO_ASYNC_BF,
    ALGO_SYNC_BF,
} ALGORITHM;


int read_result(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors);
int store_result_soft(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors);
int store_result_hard(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors);
int store_matrix_soft(int seed, int n_nodes, int n_edges, int max_weight, FlatMatrix *fm);

#endif
