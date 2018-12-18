// main file for serial versions
#include "benchmarks.h"
#include "resultr.h"

// function declarations
void print_path(size_t *predecessors, size_t idx);

int main(int argc, char **argv) {

    double start_wall, end_wall, cpu;

    // arguments we need are the number of nodes and number of edges
    if (argc != 4) {
        printf("Usage: serial [n_nodes] [n_edges] [max_weight]\n");
        exit(1);
    }

    debug_init();

    size_t n_nodes = atoi(argv[1]);
    size_t n_edges = atoi(argv[2]);
    int max_weight = atoi(argv[3]);

    FlatMatrix *adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
    if (adj_matrix == NULL) {
        exit(1);
    }


    // function signatures should look like
    // int shortest_path(adj_matrix, n_nodes, source, WEIGHT *distances, int **paths)
    // distances and paths will be the outputs
    //  distances[i] will be the cost of the shortest path going from source to i
    //  paths[i] will be the actual path. Each row in paths will have length n_nodes and will
    //      essentially be "null-terminated" with the element i. If distances[i] is infinity, then
    //      paths[i] is invalid.

    //print_array(adj_matrix, n_nodes);

    size_t *dijkstra_predecessors = calloc(n_nodes, sizeof(size_t));
    size_t *bf_predecessors = calloc(n_nodes, sizeof(size_t));

    WEIGHT *dijkstra_distances = calloc(n_nodes, sizeof(WEIGHT));


    timing(&start_wall, &cpu);
    serial_dijkstra(adj_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    dijkstra_distances,
                    dijkstra_predecessors);

    timing(&end_wall, &cpu);
    printf("Dijkstra's time: %.4f\n", end_wall - start_wall);

    WEIGHT *bf_distances = calloc(n_nodes, sizeof(WEIGHT));

    timing(&start_wall, &cpu);
    serial_bellman_ford(adj_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    bf_distances,
                    bf_predecessors);

    timing(&end_wall, &cpu);
    printf("BF's time: %.4f\n", end_wall - start_wall);

    // make sure they're the same!
    // TODO: replace this with a norm
    for (size_t i = 0; i < n_nodes; i++) {
        if (bf_distances[i] != dijkstra_distances[i]) {
            printf("Disagreement at index %zd! Dijkstras %d BF %d\n", i, dijkstra_distances[i], bf_distances[i]);
            printf("\tDijkstra's path:\n\t\t");
            print_path(dijkstra_predecessors, i);
            printf("\n\tBF's path:\n\t\t");
            print_path(bf_predecessors, i);
            printf("\n\n");
        }
    }

    double l2 = l2_norm(dijkstra_distances, bf_distances, n_nodes);
    printf("\n\nL2 Norm: %lf\n", l2);

    // now write using resultr
    store_result_soft(SEED, n_nodes, n_edges, max_weight, ALGO_SER_DIJKSTRA, dijkstra_distances, dijkstra_predecessors);
    store_result_soft(SEED, n_nodes, n_edges, max_weight, ALGO_SER_BF, bf_distances, bf_predecessors);

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////
    free(dijkstra_predecessors);
    free(bf_predecessors);
    free(dijkstra_distances);
    free(bf_distances);
    flat_matrix_free(adj_matrix);

    return 0;
}

