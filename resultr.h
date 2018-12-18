#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "flat_matrix.h"

typedef enum {
    ALGO_SER_DIJKSTRA,
    ALGO_SER_BF,
    ALGO_PAR_DIJKSTRA
} ALGORITHM;


int read_result(int seed, size_t n_nodes, size_t n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, size_t *predecessors);
int store_result_soft(int seed, size_t n_nodes, size_t n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, size_t *predecessors);
int store_result_hard(int seed, size_t n_nodes, size_t n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, size_t *predecessors);
