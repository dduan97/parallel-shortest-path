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


    // function signatures should look like
    // int shortest_path(adj_matrix, n_nodes, source, int *path_costs, int **paths)
    // path_costs and paths will be the outputs
    //  path_costs[i] will be the cost of the shortest path going from source to i
    //  paths[i] will be the actual path. Each row in paths will have length n_nodes and will
    //      essentially be "null-terminated" with the element i. If path_costs[i] is infinity, then
    //      paths[i] is invalid.

    print_array(adj_matrix, n_nodes);

    free_array(adj_matrix, n_nodes);

    return 0;
}

// returns 0 on success, -1 on failure for whatever reason.
int serial_dijkstra(int **adj_matrix, int n_nodes, int src, int *path_costs, int **paths) {

    return 1;
}
