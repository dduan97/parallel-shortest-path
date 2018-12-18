#ifndef __BENCHMARKS_H__
#define __BENCHMARKS_H__

#include <stdio.h>
#include <limits.h>

#include "helpers.h"
#include "min_queue.h"
#include "flat_matrix.h"

int serial_dijkstra(FlatMatrix *adj_matrix, int n_nodes, unsigned long n_edges, int src, WEIGHT *distances, int *predecessors);
int serial_bellman_ford(FlatMatrix *adj_matrix, int n_nodes, unsigned long n_edges, int src, WEIGHT *distances, int *predecessors);

#endif
