// main file for serial versions
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

#include "helpers.h"
#include "min_queue.h"

// a global variable for debug mode
int DEBUG_MODE = 0;

// debug statements
void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (DEBUG_MODE) {
        printf("[DEBUG]: ");
        vprintf(fmt, args);
    }
}


// function declarations
int serial_dijkstra(int **adj_matrix, int n_nodes, int src, int *path_costs, int **paths);



int main(int argc, char **argv) {

    // arguments we need are the number of nodes and number of edges
    if (argc != 4) {
        printf("Usage: serial [n_nodes] [n_edges] [max_weight]\n");
        exit(1);
    }

    // now we check for debug mode
    if (getenv("DEBUG")) {
        DEBUG_MODE = 1;
        printf("INFO: running in debug mode...\n");
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

    //print_array(adj_matrix, n_nodes);

    int *path_costs = calloc(n_nodes, sizeof(int));
    serial_dijkstra(adj_matrix,
                    n_nodes,
                    0,
                    path_costs,
                    NULL);

    printf("RESULTS!!!! to node 0\n");
    for (int i = 0; i < n_nodes; i++) {
        printf("%d ", path_costs[i]);
    }
    printf("\n");

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////

    free(path_costs);
    free_array(adj_matrix, n_nodes);

    return 0;
}

// returns 0 on success, -1 on failure for whatever reason.
int serial_dijkstra(int **adj_matrix, int n_nodes, int src, int *path_costs, int **paths) {
    // first we need a distance vector type thing
    int *previous = malloc(n_nodes *sizeof(int));
    MQNode **mqns = malloc(n_nodes * sizeof(MQNode)); // oh boy
    path_costs[src] = 0;

    MinQueue *mq = mqueue_init(n_nodes);

    for (int v = 0; v < n_nodes; v++) {
        if (v != src) {
            path_costs[v] = INT_MAX;  // doesn't feel too kosher but we're going with it
        }
        previous[v] = -1;
        MQNode *mqn = malloc(sizeof(MQNode));
        mqns[v] = mqn;
        mqn->key = v;
        mqn->val = path_costs[v];
        mqueue_insert(mq, mqn);
    }

    while (!mqueue_is_empty(mq)) {
        MQNode *mqn = mqueue_pop_min(mq);
        int v = mqn->key;
        debugf("Popped node %d\n", v);
        for (int n = 0; n < n_nodes; n++) {
            if (!adj_matrix[n][v]) {
                continue;
            }
            long alt_dist = path_costs[v] + adj_matrix[n][v];
            debugf("For node %d, alt_dist %ld, path_costs %d, adj_matrix %d\n", n, alt_dist, path_costs[n], adj_matrix[n][v]);
            if (path_costs[v] != INT_MAX && alt_dist < path_costs[n]) {
                path_costs[n] = (int) alt_dist;
                previous[n] = v;
                mqueue_update_val(mq, mqns[n], alt_dist);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////
    // CLEAN UP
    ///////////////////////////////////////////////////////////////////
    mqueue_free(mq, 1);
    free(mqns);
    return 0;
}
