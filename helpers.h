#define SEED 21897
#define WEIGHT int

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>


WEIGHT **gen_graph(size_t n_nodes, size_t n_edges, int max_weight);

// function for pretty printing a square 2-d array
void print_array(WEIGHT **arr, size_t dim);

void free_array(WEIGHT **arr, size_t dim);

// timing routines
void timing(double* wcTime, double* cpuTime);
void timing_(double* wcTime, double* cpuTime);

double l2_norm(WEIGHT *arr1, WEIGHT *arr2, size_t len);
