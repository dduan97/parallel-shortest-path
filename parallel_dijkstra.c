#include <stdio.h>
#include <limits.h>
#include <mpi.h>

#include "helpers.h"
#include "benchmarks.h"

enum MPI_TAG {
    TAG_KEY,
    TAG_VAL
};

static void pprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("[RANK %d]: ", rank);
    vprintf(fmt, args);
}

MQNode DUMMY = {-1, -1, -1};

int parallel_dijkstra(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);

int main(int argc, char **argv) {

    double start_wall, end_wall, cpu;

    MPI_Init(&argc, &argv);

    // arguments we need are the number of nodes and number of edges
    if (argc != 4) {
        printf("Usage: serial [n_nodes] [n_edges] [max_weight]\n");
        exit(1);
    }

    debug_init();

    size_t n_nodes = atoi(argv[1]);
    size_t n_edges = atoi(argv[2]);
    int max_weight = atoi(argv[3]);

    FlatMatrix *adj_matrix;
    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    // master proc should split the graph up and distribute to each node
    // also each proc should have their own version of distances[] and predecessors[]

    // now we get our cluster.
    // ASSUME that num_procs | n_nodes
    int nodes_per_proc = n_nodes / n_procs;
    // each node will have a matrix with nodes_per_proc rows and n_nodes cols
    FlatMatrix *per_node_matrix = flat_matrix_init(n_nodes, nodes_per_proc);
    // global distances and global predecessors for gathering
    WEIGHT *global_distances = NULL;
    size_t *global_predecessors = NULL;

    pprintf("About to have master generate stuff\n");
    // the master proc will generate the graph
    if (rank == 0) {
        // allocate global distances and predecessors
        global_distances = calloc(n_nodes, sizeof(WEIGHT));
        global_predecessors = calloc(n_nodes, sizeof(size_t));


        adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
        if (adj_matrix == NULL) {
            exit(1);
        }

        pprintf("About to scatter adj\n");
        // we want to scatter. Since we store the matrix in row-major form, we can just
        // take chunks of the adjacency matrix
        int scatter_res = MPI_Scatter(adj_matrix->arr,
                    nodes_per_proc * n_nodes,
                    MPI_INT,
                    per_node_matrix->arr,
                    nodes_per_proc * n_nodes,
                    MPI_INT,
                    0,
                    MPI_COMM_WORLD);
        pprintf("Scattered adj\n");
        if (scatter_res) {
            pprintf("Error when scattering!\n");
        }
    }

    // function signatures should look like
    // int shortest_path(adj_matrix, n_nodes, source, WEIGHT *distances, int **paths)
    // distances and paths will be the outputs
    //  distances[i] will be the cost of the shortest path going from source to i
    //  paths[i] will be the actual path. Each row in paths will have length n_nodes and will
    //      essentially be "null-terminated" with the element i. If distances[i] is infinity, then
    //      paths[i] is invalid.

    //print_array(adj_matrix, n_nodes);

    // each node has its own predecessors and distances arrays
    size_t *dijkstra_predecessors = calloc(nodes_per_proc, sizeof(size_t));

    WEIGHT *dijkstra_distances = calloc(nodes_per_proc, sizeof(WEIGHT));


    pprintf("About to call parallel dijkstra\n");
    timing(&start_wall, &cpu);
    parallel_dijkstra(per_node_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    dijkstra_distances,
                    dijkstra_predecessors);

    timing(&end_wall, &cpu);
    pprintf("Dijkstra's time: %.4f\n", end_wall - start_wall);

    // now we gather the results
    MPI_Gather(dijkstra_distances, nodes_per_proc, MPI_INT, global_distances, nodes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {

        // and now we have the global distances!
        pprintf("DISTANCES!\n");
        for(int i = 0; i < n_nodes; i++) {
            printf("%d ", global_distances[i]);
        }
        printf("\n");
    }

    /*WEIGHT *serial_distances = calloc(n_nodes, sizeof(WEIGHT));*/

    /*timing(&start_wall, &cpu);*/
    /*serial_dijkstra(adj_matrix,*/
                    /*n_nodes,*/
                    /*n_edges,*/
                    /*0,*/
                    /*serial_distances,*/
                    /*serial_predecessors);*/

    /*timing(&end_wall, &cpu);*/
    /*printf("BF's time: %.4f\n", end_wall - start_wall);*/

    // use the serial version to verify results

    /*double l2 = l2_norm(dijkstra_distances, serial_distances, n_nodes);*/
    /*printf("\n\nL2 Norm: %lf\n", l2);*/

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////
    free(dijkstra_predecessors);
    /*free(serial_predecessors);*/
    free(dijkstra_distances);
    /*free(serial_distances);*/
    if (rank == 0) {
        flat_matrix_free(adj_matrix);
    }
    flat_matrix_free(per_node_matrix);

    MPI_Finalize();

    return 0;
}

int parallel_dijkstra(FlatMatrix *adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
    // same thing as serial, except...
    // 1. Each processor gets assigned a "cluster" of nodes and maintains their own min heap
    //      WE WILL ASSUME THAT THE NUMBER OF PROCESSORS DIVIDES THE NUMBER OF NODES
    // 2. When we want our global min, each node reports their local min
    //      and we aggregate to get our global min
    // 3. We advertise the global min, and each processor updates their own nodes to check if they
    //      need updating
    // 4. Repeat until they are all empty
    //
    // For clarity, I will have one master node coordinating the action (as well as doing some calculations)
    //
    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    // now we get our cluster.
    // ASSUME that num_procs | n_nodes
    size_t nodes_per_proc = n_nodes / n_procs;
    size_t offset = nodes_per_proc * rank;


    // buffers for the keys/vals for the min select
    WEIGHT *gather_vals = calloc(n_procs, sizeof(WEIGHT));
    WEIGHT *gather_keys = calloc(n_procs, sizeof(size_t));

    MQNode **mqns = malloc(nodes_per_proc * sizeof(MQNode)); // oh boy
    MinQueue *mq = mqueue_init(nodes_per_proc);

    // Everything stored in the min queue is in global terms
    // initialize all of the distances into the min queue
    for (size_t v = 0; v < nodes_per_proc; v++) {
        // each node keeps track of its own min queue
        if (v + offset == src) {
            distances[src - offset] = 0;
        } else {
            distances[v] = INT_MAX;
        }
        predecessors[v] = -1;
        MQNode *mqn = malloc(sizeof(MQNode));
        mqns[v] = mqn;
        mqn->key = v + offset;
        mqn->val = distances[v];
        mqueue_insert(mq, mqn);
    }

    while (1) {
        // now we get the global min. Each one checks its local min
        MQNode *local_min = mqueue_peek_min(mq);
        // if the node is empty, we will send -1, -1
        if (local_min == NULL) {
            local_min = &DUMMY;
        }


        pprintf("About to allgather\n");
        // now we do allgather. Once for the keys and once for the vals
        MPI_Allgather(&local_min->key, 1, MPI_LONG, gather_keys, 1, MPI_LONG, MPI_COMM_WORLD);
        MPI_Allgather(&local_min->val, 1, MPI_INT, gather_vals, 1, MPI_INT, MPI_COMM_WORLD);
        pprintf("Allgathered\n");

        // now we get the local min. To ensure consistency, we pick the min from the latest
        // process if there are ties.
        int min_proc = 0;
        WEIGHT min_val = gather_vals[0];
        for (int i = 1; i < n_procs; i++) {
            if (gather_vals[i] != -1 && gather_vals[i] <= min_val) {
                min_proc= i;
                min_val = gather_vals[i];
            }
        }
        size_t min_node = gather_keys[min_proc];

        // check if we have nothing left
        if (min_val == -1) {
            break;
        }

        // reinsert if we didn't choose our own to be the min
        if (min_proc == rank) {
            pprintf("Chose min_node %d val %d\n", min_node, min_val);
            mqueue_pop_min(mq);
            // otherwise we update the distances thing
            distances[min_node - offset] = min_val;
        }

        // now each proc updates their own mqueue based on the min val that was chosen
        for (int i = 0; i < nodes_per_proc; i++) {
            pprintf("i, min_node %d, %d\n", i, min_node);
            if (!flat_matrix_get(adj_matrix, i, min_node)) {
                continue;
            }
            size_t alt_dist = min_val + flat_matrix_get(adj_matrix, i, min_node);
            // debugf("For node %d, alt_dist %ld, distances %d, adj_matrix %d\n", n, alt_dist, distances[n], flat_matrix_get(adj_matrix, n, v));
            if (min_val != INT_MAX && alt_dist < distances[i]) {
                distances[i] = (WEIGHT) alt_dist;
                predecessors[i] = min_node; // this will be globally indexed
                pprintf("Updating node %d distance to %d\n", i + offset, alt_dist);
                mqueue_update_val(mq, mqns[i], alt_dist);
            }
        }
        pprintf("Bottom of loop\n");

    }

    for (int i = 0; i < nodes_per_proc; i++) {
        pprintf("NOde %zd distance: %d\n", i + offset, distances[i]);
    }

    //////////////////////////////////////////////////////////////
    // CLEANUP
    //////////////////////////////////////////////////////////////
    free(gather_keys);
    free(gather_vals);
    mqueue_free(mq, 0);
    for (int i = 0; i < nodes_per_proc; i++) {
        free(mqns[i]);
    }
    free(mqns);

}
