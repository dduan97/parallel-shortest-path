// main file for serial versions
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

#include "helpers.h"
#include "min_queue.h"

// a global variable for debug mode
int DEBUG_MODE = 0;

// debug statements
static void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (DEBUG_MODE) {
        printf("[DEBUG]: ");
        vprintf(fmt, args);
    }
}

// function declarations
void print_path(size_t *predecessors, size_t idx);
int serial_dijkstra(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);
int serial_bellman_ford(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);

int main(int argc, char **argv) {

    double start_wall, end_wall, cpu;

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

    size_t n_nodes = atoi(argv[1]);
    size_t n_edges = atoi(argv[2]);
    int max_weight = atoi(argv[3]);

    WEIGHT **adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
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

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////
    free(dijkstra_predecessors);
    free(bf_predecessors);
    free(dijkstra_distances);
    free(bf_distances);
    free_array(adj_matrix, n_nodes);

    return 0;
}

// returns 0 on success, -1 on failure for whatever reason.
int serial_dijkstra(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
    // first we need a distance vector type thing
    MQNode **mqns = malloc(n_nodes * sizeof(MQNode)); // oh boy
    distances[src] = 0;

    MinQueue *mq = mqueue_init(n_nodes);

    for (size_t v = 0; v < n_nodes; v++) {
        if (v != src) {
            distances[v] = INT_MAX;  // doesn't feel too kosher but we're going with it
        }
        predecessors[v] = -1;
        MQNode *mqn = malloc(sizeof(MQNode));
        mqns[v] = mqn;
        mqn->key = v;
        mqn->val = distances[v];
        mqueue_insert(mq, mqn);
    }

    while (!mqueue_is_empty(mq)) {
        MQNode *mqn = mqueue_pop_min(mq);
        size_t v = mqn->key;
        debugf("Popped node %d with distance %d\n", v, distances[v]);
        for (size_t n = 0; n < n_nodes; n++) {
            if (!adj_matrix[n][v]) {
                continue;
            }
            size_t alt_dist = distances[v] + adj_matrix[n][v];
            debugf("For node %d, alt_dist %ld, distances %d, adj_matrix %d\n", n, alt_dist, distances[n], adj_matrix[n][v]);
            if (distances[v] != INT_MAX && alt_dist < distances[n]) {
                distances[n] = (WEIGHT) alt_dist;
                predecessors[n] = v;
                debugf("Updating node %d distance to %d\n", n, alt_dist);
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

// returns 0 on success, -1 on failure for whatever reason.
// TODO: fix BF. returning 0 for all distances.
int serial_bellman_ford(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
    // preprocess once to convert adjacency matrix to edge list
    typedef struct {
        size_t n1;
        size_t n2;
        WEIGHT weight;
    } Edge;

    Edge *edges= calloc(n_edges, sizeof(Edge));

    size_t e_i = 0;
    for (size_t i = 0; i < n_nodes; i++) {
        for (size_t j = i; j < n_nodes; j++) {
            if (adj_matrix[i][j]) {
                Edge e = {i, j, adj_matrix[i][j]};
                edges[e_i++] = e;
            }
            if (e_i == n_edges) break;
        }
    }


    // now that we have the edge list...
    for (size_t i = 0; i < n_nodes; i++) {
        distances[i] = INT_MAX;
        predecessors[i] = -1;
    }

    distances[src] = 0;

    for (size_t v = 1; v < n_nodes; v++){
        for (size_t e = 0; e < n_edges; e++) {
            size_t u = edges[e].n1;
            size_t v = edges[e].n2;
            WEIGHT w = edges[e].weight;
            // check if v can go through u instead
            if (distances[u] != INT_MAX && distances[u] + w < distances[v]) {
                debugf("FOUND BETTER PATH FOR %d through %d (old dist: %d new %d)\n", v, u, distances[v], distances[u] + w);
                distances[v] = distances[u] + w;
                predecessors[v] = u;
            }
            if (distances[v] != INT_MAX && distances[v] + w < distances[u]) {
                distances[u] = distances[v] + w;
                predecessors[u] = v;
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////
    // CLEAN UP
    ////////////////////////////////////////////////////////////////////////////////
    free(edges);

    return 0;
}


void print_path(size_t *predecessors, size_t idx) {
    while (idx != -1) {
        printf("%zd ", idx);
        idx = predecessors[idx];
    }
}
