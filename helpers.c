// helper file to generate a random graph or something
// takes in the desired number of nodes and desired number of edges and
// returns a randomly generated graph using those parameters
// right now we're using integer weights
#include "helpers.h"

FlatMatrix *gen_graph(int n_nodes, unsigned long n_edges, int max_weight) {

    unsigned long max_edges = (n_nodes - 1) * n_nodes / 2;
    if (n_edges > max_edges) {
        printf("Too many edges!\n");
        return NULL;
    }
    srand(SEED);
    // initialize the adjacency matrix as a 2-D array
    FlatMatrix *adj_matrix = flat_matrix_init(n_nodes, n_nodes);

    // now we randomly select the adjacencies to put in the matrix
    unsigned long e = 0;
    while (e < n_edges) {
        // this isn't uniform but it sohuld suffice for our purposes
        int n1 = (int) rand()*1.0 / RAND_MAX * n_nodes;
        int n2 = (int) rand()*1.0 / RAND_MAX * n_nodes;


        // don't want to repeat an edge or put an edge between the same nodes
        if (n1 == n2
                || flat_matrix_get(adj_matrix, n1, n2) != 0) {
            continue;
        }

        // generate the weights, from 1 til max_weight (inclusive)
        WEIGHT weight = (WEIGHT) (rand() * 1.0 / RAND_MAX * max_weight) + 1;

        flat_matrix_set(adj_matrix, n1, n2, weight);
        flat_matrix_set(adj_matrix, n2, n1, weight);
        e++;
    }

    return adj_matrix;
}


/*// function for pretty printing a square 2-d array*/
/*void print_array(WEIGHT**arr, int dim) {*/
    /*for (int i = 0; i < dim; i++) {*/
        /*for (int j = 0; j < dim; j++) {*/
            /*printf("%4d ", arr[i][j]);*/
        /*}*/
        /*printf("\n");*/
    /*}*/
/*}*/


/*void free_array(WEIGHT**arr, int dim) {*/
    /*for (int i = 0; i < dim; i++) {*/
        /*free(arr[i]);*/
    /*}*/
    /*free(arr);*/
/*}*/


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


// error norm. I guess we'll use l2
double l2_norm(WEIGHT *arr1, WEIGHT *arr2, int len) {
    WEIGHT err = 0;
    for (int i = 0; i < len; i++) {
        err += (arr1[i] - arr2[i]) * (arr1[i] - arr2[i]);
    }
    return sqrt((double) err);
}

void debug_init() {
    // now we check for debug mode
    if (getenv("DEBUG")) {
        DEBUG_MODE = 1;
        printf("INFO: running in debug mode...\n");
    }
}

// debug statements
void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (DEBUG_MODE) {
        printf("[DEBUG]: ");
        vprintf(fmt, args);
    }
}

