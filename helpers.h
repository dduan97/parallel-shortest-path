#define SEED 12345
#define WEIGHT int

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>


WEIGHT **gen_graph(int n_nodes, int n_edges, int max_weight);

// function for pretty printing a square 2-d array
void print_array(WEIGHT **arr, int dim);

void free_array(WEIGHT **arr, int dim);

// timing routines
void timing(double* wcTime, double* cpuTime);
void timing_(double* wcTime, double* cpuTime);

