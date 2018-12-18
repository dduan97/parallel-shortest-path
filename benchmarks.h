#ifndef __BENCHMARKS_H__
#define __BENCHMARKS_H__

#include <stdio.h>
#include <limits.h>

#include "helpers.h"
#include "min_queue.h"
#include "flat_matrix.h"

int serial_dijkstra(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);
int serial_bellman_ford(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);

#endif
