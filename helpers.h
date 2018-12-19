#ifndef __HELPERS_H__
#define __HELPERS_H__

#define SEED 2017

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "flat_matrix.h"
#include "resultr.h"

static int DEBUG_MODE = 0;

FlatMatrix *gen_graph(int n_nodes, unsigned long n_edges, int max_weight);

// function for pretty printing a square 2-d array
void print_array(WEIGHT **arr, int dim);

void free_array(WEIGHT **arr, int dim);

// timing routines
void timing(double* wcTime, double* cpuTime);
void timing_(double* wcTime, double* cpuTime);

double l2_norm(WEIGHT *arr1, WEIGHT *arr2, int len);
void debug_init();
void debugf(const char *fmt, ...);

#endif
