#include "benchmarks.h"

// returns 0 on success, -1 on failure for whatever reason.
int serial_dijkstra(FlatMatrix *adj_matrix, int n_nodes, unsigned long n_edges, int dest, WEIGHT *distances, int *next_hops) {
    // first we need a distance vector type thing
    MQNode **mqns = malloc(n_nodes * sizeof(MQNode)); // oh boy
    distances[dest] = 0;

    MinQueue *mq = mqueue_init(n_nodes);

    // initialize all of the distances into the min queue
    for (int v = 0; v < n_nodes; v++) {
        if (v != dest) {
            distances[v] = INT_MAX;  // doesn't feel too kosher but we're going with it
        }
        next_hops[v] = -1;
        MQNode *mqn = malloc(sizeof(MQNode));
        mqns[v] = mqn;
        mqn->key = v;
        mqn->val = distances[v];
        mqueue_insert(mq, mqn);
    }

    while (!mqueue_is_empty(mq)) {
        MQNode *mqn = mqueue_pop_min(mq);
        int v = mqn->key;
        debugf("Popped node %d with distance %d\n", v, distances[v]);
        for (int n = 0; n < n_nodes; n++) { // iterate through each in neighbor of this node
            if (!flat_matrix_get(adj_matrix, n, v)) {
                continue;
            }
            WEIGHT alt_dist = distances[v] + flat_matrix_get(adj_matrix, n, v);
            debugf("For node %d, alt_dist %ld, distances %d, adj_matrix %d\n", n, alt_dist, distances[n], flat_matrix_get(adj_matrix, n, v));
            if (distances[v] != INT_MAX && alt_dist < distances[n]) {
                distances[n] = (WEIGHT) alt_dist;
                next_hops[n] = v;
                debugf("Updating node %d distance to %d\n", n, alt_dist);
                mqueue_update_val(mq, mqns[n], alt_dist);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////
    // CLEAN UP
    ///////////////////////////////////////////////////////////////////
    mqueue_free(mq, 0);
    for (int i = 0; i < n_nodes; i++) {
        free(mqns[i]);
    }
    free(mqns);
    return 0;
}

// returns 0 on success, -1 on failure for whatever reason.
int serial_bellman_ford(FlatMatrix *adj_matrix, int n_nodes, unsigned long n_edges, int dest, WEIGHT *distances, int *next_hops) {
    // preprocess once to convert adjacency matrix to edge list
    typedef struct {
        int n1;
        int n2;
        WEIGHT weight;
    } Edge;

    // each edj struct is a directed edge from n1 to n2
    Edge *edges= calloc(n_edges, sizeof(Edge));

    int e_i = 0;
    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < n_nodes; j++) {
            if (flat_matrix_get(adj_matrix, i, j)) {
                Edge e = {i, j, flat_matrix_get(adj_matrix, i, j)};
                edges[e_i++] = e;
            }
        }
    }


    // now that we have the edge list...
    for (int i = 0; i < n_nodes; i++) {
        distances[i] = INT_MAX;
        next_hops[i] = -1;
    }

    distances[dest] = 0;

    for (int n = 0; n < n_nodes; n++){
        for (unsigned long e = 0; e < n_edges; e++) {
            // edge from u to v
            int u = edges[e].n1;
            int v = edges[e].n2;
            WEIGHT w = edges[e].weight;
            // check if u can go through v instead
            debugf("Checking for a path from node %d through node %d\n", u, v);
            debugf("distances[%d}: %d distances[%d]: %d\n", u, distances[u], v, distances[v]);
            if (distances[v] != INT_MAX && distances[v] + w < distances[u]) {
                debugf("Found! Old distances[%d]: %d. New: %d\n", u, distances[u], distances[v] + w);
                distances[u] = distances[v] + w;
                next_hops[u] = v;
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////
    // CLEAN UP
    ////////////////////////////////////////////////////////////////////////////////
    free(edges);

    return 0;
}


void print_path(int *next_hops, int idx) {
    while (idx != -1) {
        printf("%d ", idx);
        idx = next_hops[idx];
    }
}
