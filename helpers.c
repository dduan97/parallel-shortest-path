// helper file to generate a random graph or something
// takes in the desired number of nodes and desired number of edges and
// returns a randomly generated graph using those parameters
// right now we're using integer weights
#include "helpers.h"

int **gen_graph(int n_nodes, int n_edges, int max_weight) {

    if (n_edges > (n_nodes) * (n_nodes - 1) / 2) {
        printf("Too many edges!\n");
        return NULL;
    }
    srand(SEED);
    // initialize the adjacency matrix as a 2-D array
    int **adj_matrix = malloc(n_nodes * sizeof(int*));
    for (int i = 0; i < n_nodes; i++) {
        adj_matrix[i] = calloc(n_nodes, sizeof(int));
    }

    // now we randomly select the adjacencies to put in the matrix
    int e = 0;
    while (e < n_edges) {
        // this isn't uniform but it sohuld suffice for our purposes
        int n1 = rand() % n_nodes;
        int n2 = rand() % n_nodes;


        // don't want to repeat an edge or put an edge between the same nodes
        if (n1 == n2
                || adj_matrix[n1][n2] != 0) {
            continue;
        }

        // generate the weights, from 1 til max_weight (inclusive)
        int weight = rand() % max_weight + 1;

        adj_matrix[n1][n2] = adj_matrix[n2][n1] = weight;
        e++;
    }

    return adj_matrix;
}


// function for pretty printing a square 2-d array
void print_array(int **arr, int dim) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
}


void free_array(int **arr, int dim) {
    for (int i = 0; i < dim; i++) {
        free(arr[i]);
    }
    free(arr);
}
