#include "benchmarks.h"

// returns 0 on success, -1 on failure for whatever reason.
int serial_dijkstra(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
    // first we need a distance vector type thing
    MQNode **mqns = malloc(n_nodes * sizeof(MQNode)); // oh boy
    distances[src] = 0;

    MinQueue *mq = mqueue_init(n_nodes);

    // initialize all of the distances into the min queue
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
        for (size_t n = 0; n < n_nodes; n++) { // iterate through each neighbor of that node
            if (!flat_matrix_get(adj_matrix, n, v)) {
                continue;
            }
            size_t alt_dist = distances[v] + flat_matrix_get(adj_matrix, n, v);
            debugf("For node %d, alt_dist %ld, distances %d, adj_matrix %d\n", n, alt_dist, distances[n], flat_matrix_get(adj_matrix, n, v));
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
    mqueue_free(mq, 0);
    free(mqns);
    return 0;
}

// returns 0 on success, -1 on failure for whatever reason.
int serial_bellman_ford(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
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
            if (flat_matrix_get(adj_matrix, i, j)) {
                Edge e = {i, j, flat_matrix_get(adj_matrix, i, j)};
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