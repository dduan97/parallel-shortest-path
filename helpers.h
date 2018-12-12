#define SEED 12345

#include <stdlib.h>
#include <stdio.h>

int **gen_graph(int n_nodes, int n_edges, int max_weight);

// function for pretty printing a square 2-d array
void print_array(int **arr, int dim);

void free_array(int **arr, int dim);
