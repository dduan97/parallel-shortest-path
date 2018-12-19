#include <stdio.h>
#include <limits.h>
#include <mpi.h>

#include "helpers.h"
#include "benchmarks.h"
#include "resultr.h"

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

int parallel_dijkstra(FlatMatrix *adj_matrix, int n_nodes, int n_edges, int src, WEIGHT *distances, int *next_hops);

int main(int argc, char **argv) {

    double start_wall, end_wall, cpu;

    MPI_Init(&argc, &argv);

    // arguments we need are the number of nodes and number of edges
    if (argc != 4) {
        printf("Usage: serial [n_nodes] [n_edges] [max_weight]\n");
        exit(1);
    }


    debug_init();

    int n_nodes = atoi(argv[1]);
    unsigned long n_edges = atoi(argv[2]);
    int max_weight = atoi(argv[3]);

    FlatMatrix *adj_matrix;
    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    // master proc should split the graph up and distribute to each node
    // also each proc should have their own version of distances[] and next_hops[]

    // now we get our cluster.
    // ASSUME that num_procs | n_nodes
    int nodes_per_proc = n_nodes / n_procs;
    // each node will have a matrix with nodes_per_proc rows and n_nodes cols
    FlatMatrix *per_node_matrix = flat_matrix_init(n_nodes, nodes_per_proc);
    // global distances and global next_hops for gathering
    WEIGHT *global_distances = NULL;
    int *global_next_hops = NULL;

    // the master proc will generate the graph
    if (rank == 0) {
        // allocate global distances and next_hops
        global_distances = calloc(n_nodes, sizeof(WEIGHT));
        global_next_hops = calloc(n_nodes, sizeof(int));


        adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
        if (adj_matrix == NULL) {
            exit(1);
        }

        //flat_matrix_print(adj_matrix);
        /*for (int i = 0; i < n_nodes * n_nodes; i++) {*/
            /*printf("%d ", adj_matrix->arr[i]);*/
        /*}*/
    }
    MPI_Barrier(MPI_COMM_WORLD);
    timing(&start_wall, &cpu);

    int send_per_proc = nodes_per_proc * n_nodes;
    int *send_buf = (rank == 0) ? adj_matrix->arr : &send_per_proc;
    // we want to scatter. Since we store the matrix in row-major form, we can just
    // take chunks of the adjacency matrix
    int scatter_res = MPI_Scatter(
            send_buf,
            send_per_proc,
            MPI_INT,
            per_node_matrix->arr,
            send_per_proc,
            MPI_INT,
            0,
            MPI_COMM_WORLD);
    if (scatter_res) {
        pprintf("Error when scattering!\n");
    }

    // function signatures should look like
    // int shortest_path(adj_matrix, n_nodes, source, WEIGHT *distances, int **paths)
    // distances and paths will be the outputs
    //  distances[i] will be the cost of the shortest path going from source to i
    //  paths[i] will be the actual path. Each row in paths will have length n_nodes and will
    //      essentially be "null-terminated" with the element i. If distances[i] is infinity, then
    //      paths[i] is invalid.

    //print_array(adj_matrix, n_nodes);

    // each node has its own next_hops and distances arrays
    int *dijkstra_next_hops = calloc(nodes_per_proc, sizeof(int));

    WEIGHT *dijkstra_distances = calloc(nodes_per_proc, sizeof(WEIGHT));


    parallel_dijkstra(per_node_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    dijkstra_distances,
                    dijkstra_next_hops);


    // now we gather the results
    MPI_Gather(dijkstra_distances, nodes_per_proc, MPI_INT, global_distances, nodes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(dijkstra_next_hops, nodes_per_proc, MPI_INT, global_next_hops, nodes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    timing(&end_wall, &cpu);
    pprintf("Dijkstra's time: %.4f\n", end_wall - start_wall);
    // for comparison, get results from serial dijsktra
    if (rank == 0) {
        // save the results
        int store_res = store_result_soft(SEED, n_nodes, n_edges, max_weight, ALGO_PAR_DIJKSTRA, global_distances, global_next_hops);
        if (store_res == -1) {
            printf("Could not store result!\n");
        }

        WEIGHT *ser_distances = calloc(n_nodes, sizeof(WEIGHT));
        int *ser_next_hops = calloc(n_nodes, sizeof(int));
        int res = read_result(SEED, n_nodes, n_edges, max_weight, ALGO_SER_DIJKSTRA, ser_distances, ser_next_hops);
        if (res == -1) {
            pprintf("Could not read past result!\n");
        } else {
            double l2 = l2_norm(global_distances, ser_distances, n_nodes);
            printf("L2 norm with serial dijkstra: %lf\n", l2);

            /*// and now we have the global distances!*/
            /*pprintf("DIJKSTRA DISTANCES!\n");*/
            /*for(int i = 0; i < n_nodes; i++) {*/
                /*printf("%d ", global_distances[i]);*/
            /*}*/
            /*printf("\n");*/

            /*// and now we have the global distances!*/
            /*pprintf("SERIAL DISTANCES!\n");*/
            /*for(int i = 0; i < n_nodes; i++) {*/
                /*printf("%d ", global_distances[i]);*/
            /*}*/
            /*printf("\n");*/

        }
        free(ser_distances);
        free(ser_next_hops);
    }

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////
    free(dijkstra_next_hops);
    /*free(serial_next_hops);*/
    free(dijkstra_distances);
    /*free(serial_distances);*/
    if (rank == 0) {
        flat_matrix_free(adj_matrix);
        free(global_distances);
        free(global_next_hops);
    }
    flat_matrix_free(per_node_matrix);

    MPI_Finalize();

    return 0;
}

int parallel_dijkstra(FlatMatrix *adj_matrix, int n_nodes, int n_edges, int src, WEIGHT *distances, int *next_hops) {

    //pprintf("PER_NODE MATRIX!!!!!\n");
    //flat_matrix_print(adj_matrix);
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
    int nodes_per_proc = n_nodes / n_procs;
    int offset = nodes_per_proc * rank;


    // buffers for the keys/vals for the min select
    WEIGHT *gather_vals = calloc(n_procs, sizeof(WEIGHT));
    WEIGHT *gather_keys = calloc(n_procs, sizeof(int));

    MQNode **mqns = malloc(nodes_per_proc * sizeof(MQNode)); // oh boy
    MinQueue *mq = mqueue_init(nodes_per_proc);

    // Everything stored in the min queue is in global terms
    // initialize all of the distances into the min queue
    for (int v = 0; v < nodes_per_proc; v++) {
        // each node keeps track of its own min queue
        if (v + offset == src) {
            distances[src - offset] = 0;
        } else {
            distances[v] = INT_MAX;
        }
        next_hops[v] = -1;
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


        //pprintf("LOCAL MIN: %d %d\n", local_min->key, local_min->val);
        // now we do allgather. Once for the keys and once for the vals
        MPI_Allgather(&local_min->key, 1, MPI_INT, gather_keys, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather(&local_min->val, 1, MPI_INT, gather_vals, 1, MPI_INT, MPI_COMM_WORLD);

        // now we get the local min. To ensure consistency, we pick the min from the latest
        // process if there are ties.
        int min_proc = 0;
        WEIGHT min_val = gather_vals[0];
        for (int i = 1; i < n_procs; i++) {
            if ((gather_vals[i] != -1 && gather_vals[i] <= min_val)
                    || min_val == -1) {
                min_proc= i;
                min_val = gather_vals[i];
                //pprintf("SETTING min_proc %d min_val %zd", min_proc, min_val);
            }
        }
        int min_node = gather_keys[min_proc];

        // check if we have nothing left
        if (min_val == -1) {
            break;
        }

        //pprintf("CHOSEN MIN: key, val (%zd, %d) from proc %d\n", min_node, min_val, min_proc);
        if (min_proc == rank) {
            mqueue_pop_min(mq);
            distances[min_node - offset] = min_val;
        }

        // now each proc updates their own mqueue based on the min val that was chosen
        for (int i = 0; i < nodes_per_proc; i++) {
            // pprintf("i, min_node %d, %d\n", i, min_node);
            if (!flat_matrix_get(adj_matrix, i, min_node)) {
                //pprintf("Zero found at (%d, %zd)\n", i, min_node);
                continue;
            }
            int alt_dist = min_val + flat_matrix_get(adj_matrix, i, min_node);
            // debugf("For node %d, alt_dist %ld, distances %d, adj_matrix %d\n", n, alt_dist, distances[n], flat_matrix_get(adj_matrix, n, v));
            if (min_val != INT_MAX && alt_dist < distances[i]) {
                distances[i] = (WEIGHT) alt_dist;
                next_hops[i] = min_node; // this will be globally indexed
                /*pprintf("Updating node %d distance to %d\n", i + offset, alt_dist);*/
                mqueue_update_val(mq, mqns[i], alt_dist);
            }
        }

    }

    /*for (int i = 0; i < nodes_per_proc; i++) {*/
        /*pprintf("NOde %zd distance: %d\n", i + offset, distances[i]);*/
    /*}*/

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
