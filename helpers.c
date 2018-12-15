// helper file to generate a random graph or something
// takes in the desired number of nodes and desired number of edges and
// returns a randomly generated graph using those parameters
// right now we're using integer weights
#include "helpers.h"

WEIGHT **gen_graph(size_t n_nodes, size_t n_edges, int max_weight) {

    if (n_edges > (n_nodes) * (n_nodes - 1) / 2) {
        printf("Too many edges!\n");
        return NULL;
    }
    srand(SEED);
    // initialize the adjacency matrix as a 2-D array
    WEIGHT **adj_matrix = malloc(n_nodes * sizeof(WEIGHT*));
    for (size_t i = 0; i < n_nodes; i++) {
        adj_matrix[i] = calloc(n_nodes, sizeof(WEIGHT));
    }

    // now we randomly select the adjacencies to put in the matrix
    size_t e = 0;
    while (e < n_edges) {
        // this isn't uniform but it sohuld suffice for our purposes
        size_t n1 = (size_t) rand()*1.0 / RAND_MAX * n_nodes;
        size_t n2 = (size_t) rand()*1.0 / RAND_MAX * n_nodes;


        // don't want to repeat an edge or put an edge between the same nodes
        if (n1 == n2
                || adj_matrix[n1][n2] != 0) {
            continue;
        }

        // generate the weights, from 1 til max_weight (inclusive)
        WEIGHT weight = (WEIGHT) (rand() * 1.0 / RAND_MAX * max_weight) + 1;

        adj_matrix[n1][n2] = adj_matrix[n2][n1] = weight;
        e++;
    }

    return adj_matrix;
}


// function for pretty printing a square 2-d array
void print_array(WEIGHT**arr, size_t dim) {
    for (size_t i = 0; i < dim; i++) {
        for (size_t j = 0; j < dim; j++) {
            printf("%4d ", arr[i][j]);
        }
        printf("\n");
    }
}


void free_array(WEIGHT**arr, size_t dim) {
    for (size_t i = 0; i < dim; i++) {
        free(arr[i]);
    }
    free(arr);
}


/// TIMING ROUTINES
void timing_(double* wcTime, double* cpuTime)
{
   timing(wcTime, cpuTime);
}

void timing(double* wcTime, double* cpuTime)
{
   struct timeval tp;
   struct rusage ruse;

   gettimeofday(&tp, NULL);
   *wcTime=(double) (tp.tv_sec + tp.tv_usec/1000000.0);

   getrusage(RUSAGE_SELF, &ruse);
   *cpuTime=(double)(ruse.ru_utime.tv_sec+ruse.ru_utime.tv_usec / 1000000.0);
}


// error norm. I guess we'll use l1
double l2_norm(WEIGHT *arr1, WEIGHT *arr2, size_t len) {
    WEIGHT err = 0;
    for (size_t i = 0; i < len; i++) {
        err += (arr1[i] - arr2[i]) * (arr1[i] - arr2[i]);
    }
    return sqrt((double) err);
}
