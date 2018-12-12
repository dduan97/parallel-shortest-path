// main file for serial versions
#include <stdio.h>

#include "helpers.h"

int main(int argc, char **argv) {

    // arguments we need are the number of nodes and number of edges
    if (argc != 4) {
        printf("Usage: serial [n_nodes] [n_edges] [max_weight]\n");
        exit(1);
    }

    int n_nodes = atoi(argv[1]);
    int n_edges = atoi(argv[2]);
    int max_weight = atoi(argv[3]);

    int **adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
    if (adj_matrix == NULL) {
        exit(1);
    }

    print_array(adj_matrix, n_nodes);
    free_array(adj_matrix, n_nodes);

    return 0;
}
