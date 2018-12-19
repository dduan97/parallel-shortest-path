#define _BSD_SOURCE
#include <stdio.h>
#include <limits.h>
#include <mpi.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"
#include "benchmarks.h"
#include "resultr.h"

#define ITERATIONS_TO_CONVERGE 20


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

static int node_to_proc(int node, int nodes_per_proc) {
    return (node / nodes_per_proc);
}

static int get_tag(int src_node, int dest_node, int n_nodes, int nodes_per_proc) {
    int src_v = src_node % nodes_per_proc;
    int dst_v = dest_node % nodes_per_proc;
    return src_v + nodes_per_proc * dst_v;
}

MQNode DUMMY = {-1, -1, -1};

int sync_bf(FlatMatrix *adj_matrix, int n_nodes, int n_edges, int src, WEIGHT *distances, int *next_hops);

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

    pprintf("About to have master generate stuff\n");
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
    } else {
        adj_matrix = flat_matrix_init(n_nodes, n_nodes);
    }

    // here we will just broadcast, since we need to know eaach node's neighbors in both directions
    int bcast_res = MPI_Bcast(
            adj_matrix->arr,
            n_nodes * n_nodes,
            MPI_INT,
            0,
            MPI_COMM_WORLD);
    if (bcast_res) {
        pprintf("Error when broadcasting!\n");
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
    int *sync_bf_next_hops = calloc(nodes_per_proc, sizeof(int));

    WEIGHT *sync_bf_distances = calloc(nodes_per_proc, sizeof(WEIGHT));


    timing(&start_wall, &cpu);
    sync_bf(adj_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    sync_bf_distances,
                    sync_bf_next_hops);

    timing(&end_wall, &cpu);
    pprintf("sync BF's time: %.4f\n", end_wall - start_wall);

    // now we gather the results
    MPI_Gather(sync_bf_distances, nodes_per_proc, MPI_INT, global_distances, nodes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(sync_bf_next_hops, nodes_per_proc, MPI_INT, global_next_hops, nodes_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // for comparison, get results from serial dijsktra
    if (rank == 0) {
        // save the results
        int store_res = store_result_hard(SEED, n_nodes, n_edges, max_weight, ALGO_SYNC_BF, global_distances, global_next_hops);
        if (store_res == -1) {
            printf("Could not store result!\n");
        }

        WEIGHT *ser_distances = calloc(n_nodes, sizeof(WEIGHT));
        int *ser_next_hops = calloc(n_nodes, sizeof(int));
        int res = read_result(SEED, n_nodes, n_edges, max_weight, ALGO_SER_DIJKSTRA, ser_distances, ser_next_hops);
        if (res == -1) {
            pprintf("Could not read past result!\n");
        } else {
            //double l2 = l2_norm(global_distances, ser_distances, n_nodes);
            //printf("L2 norm with serial BF: %lf\n", l2);

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
    free(sync_bf_next_hops);
    /*free(serial_next_hops);*/
    free(sync_bf_distances);
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

int sync_bf(FlatMatrix *adj_matrix, int n_nodes, int n_edges, int dest, WEIGHT *distances, int *next_hops) {

    // general workflow:
    // If a node has n neighbors, then we essentially keep n "receives" open, one for each neighbor.
    // Read those receives. If there is something in one of them, read it and reopen the receive
    //  EAch receive essentially is an updated estimate. Then we recalculate our value and send() to all neighbors
    // If we don't have an update in n loops, then we assume termiantion and just return.
    //
    // In general, each node waits for updates from their out_neighbors (downstream), and sends updates to their
    // in_neighbors (upstream)

    // a dummy mpi_request
    //

    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    // now we get our cluster.
    // ASSUME that num_procs | n_nodes
    int nodes_per_proc = n_nodes / n_procs;
    int offset = nodes_per_proc * rank;

    // initialize all the distances to infinity
    for (int i = 0; i < nodes_per_proc; i++) {
        if (offset + i == dest) {
            pprintf("Initializing destination node %d\n", offset + i);
            distances[i] = 0;
            next_hops[i] = -1;
        } else {
            distances[i] = INT_MAX;
            next_hops[i] = -1;
        }
    }

    int *n_in_neighbors = calloc(nodes_per_proc, sizeof(int));
    int *n_out_neighbors = calloc(nodes_per_proc, sizeof(int));
    for (int v = 0; v < nodes_per_proc; v++) {
        // figure out how many neighbors each node has
        // in neighbors of node v are at arr[i][v]
        for (int i = 0; i < n_nodes; i++) {
            // check if i is an in neighbor
            if (flat_matrix_get(adj_matrix, i, v + offset)) {
                n_in_neighbors[v]++;
            }
            if (flat_matrix_get(adj_matrix, v + offset, i)) {
                n_out_neighbors[v]++;
            }
        }
    }

    // each node has global estimates
    /*WEIGHT **estimates = calloc(nodes_per_proc, sizeof(WEIGHT *));*/


    int **in_neighbors = calloc(nodes_per_proc, sizeof(int *));
    int **out_neighbors = calloc(nodes_per_proc, sizeof(int *));

    // for each node, we will also have a "new neighbor estimate array"
    // that holds any updated estimates from OUT_NEIGHBORS
    WEIGHT **downstream_updates = calloc(nodes_per_proc, sizeof(WEIGHT *));

    WEIGHT **intraproc_updates = calloc(nodes_per_proc, sizeof(WEIGHT *));

    // populate the neighbor arrays and such
    for (int v = 0; v < nodes_per_proc; v++) {
        // allocate the list
        in_neighbors[v] = calloc(n_in_neighbors[v], sizeof(int));
        out_neighbors[v] = calloc(n_out_neighbors[v], sizeof(int));
        downstream_updates[v] = calloc(n_out_neighbors[v], sizeof(WEIGHT));
        intraproc_updates[v] = calloc(nodes_per_proc, sizeof(WEIGHT));

        int in_idx = 0;
        int out_idx = 0;
        for (int i = 0; i < n_nodes; i++) {
            // check if i is an in neighbor
            if (flat_matrix_get(adj_matrix, i, v + offset)) {
                in_neighbors[v][in_idx] = i;
                in_idx++;
            }
            if (flat_matrix_get(adj_matrix, v + offset, i)) {
                out_neighbors[v][out_idx] = i;
                out_idx++;
            }
        }
    }

    // send the round 0 updates
    for (int v = 0; v < nodes_per_proc; v++) {
        for (int i = 0; i < n_in_neighbors[v]; i++) {
            int n = in_neighbors[v][i];
            int proc = n / nodes_per_proc;

            // don't need to send to ourselves, just write to the array!
            if (proc == rank) {
                int dest_v = n % nodes_per_proc;
                intraproc_updates[v][dest_v] = distances[v];
                continue;
            }

            // send an upate to each one of the in neighbors with tag given by
            int tag = get_tag(v + offset, n, n_nodes, nodes_per_proc);
            MPI_Request req;
            //pprintf("proc is %d (n, nodespp) %d %d \n", proc, n, nodes_per_proc);
            MPI_Isend(&(distances[v]), 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
        }
    }

    for (int round = 1; round <= n_nodes; round++) {
        /*pprintf("Starting round %d\n", round);*/
        MPI_Barrier(MPI_COMM_WORLD);
        // wait for the round round-1 updates
        for (int v = 0; v < nodes_per_proc; v++) {
            for (int i = 0; i < n_out_neighbors[v]; i++) {
                int n = out_neighbors[v][i];
                int proc = n / nodes_per_proc;

                int downstream = -1;

                if (proc == rank) {
                    int src_v = n % nodes_per_proc;
                    downstream = intraproc_updates[src_v][v];
                } else {

                    int tag = get_tag(n, v+offset, n_nodes, nodes_per_proc);

                    MPI_Recv(&downstream, 1, MPI_INT, proc, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                // now update our estimate
                WEIGHT edge_weight = flat_matrix_get(adj_matrix, v+offset, n);
                /*pprintf("Node %d received update %d from node %d\n", v+offset, downstream, n);*/
                /*pprintf("Node %d candidate update: %d (current distance %d)\n", v + offset, downstream + edge_weight, distances[v]);*/
                if (downstream != INT_MAX && downstream != -1 && downstream + edge_weight < distances[v]) {
                    distances[v] = downstream + edge_weight;
                    next_hops[v] = n;
                }

            }
        }

        // send the updates for this round
        for (int v = 0; v < nodes_per_proc; v++) {
            for (int i = 0; i < n_in_neighbors[v]; i++) {
                int n = in_neighbors[v][i];
                int proc = n / nodes_per_proc;

                // send an upate to each one of the in neighbors with tag given by
                int tag = get_tag(v + offset, n, n_nodes, nodes_per_proc);
                MPI_Request req;
                //printf("proc is %d (n, nodespp) %d %d \n", proc, n, nodes_per_proc);
                MPI_Isend(&(distances[v]), 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &req);
                MPI_Request_free(&req);
            }
        }
    }


    /*// now we get the actual list of neighbors*/
    /*for (int v = 0; v < nodes_per_proc; v++) {*/
        /*// allocate the list*/
        /*in_neighbors[v] = calloc(n_in_neighbors[v], sizeof(int));*/
        /*out_neighbors[v] = calloc(n_out_neighbors[v], sizeof(int));*/
        /*downstream_updates[v] = calloc(n_out_neighbors[v], sizeof(WEIGHT));*/

        /*irecv_reqs[v] = calloc(n_out_neighbors[v], sizeof(MPI_Request));*/
        /*isend_reqs[v] = calloc(n_in_neighbors[v], sizeof(MPI_Request));*/

        /*int in_idx = 0;*/
        /*int out_idx = 0;*/
        /*for (int i = 0; i < n_nodes; i++) {*/
            /*// check if i is an in neighbor*/
            /*if (flat_matrix_get(adj_matrix, i, v + offset)) {*/
                /*in_neighbors[v][in_idx] = i;*/
                /*isend_reqs[v][in_idx] = calloc(1, sizeof(MPI_Request));*/
                /*in_idx++;*/
            /*}*/
            /*if (flat_matrix_get(adj_matrix, v + offset, i)) {*/
                /*out_neighbors[v][out_idx] = i;*/
                /*irecv_reqs[v][out_idx] = calloc(1, sizeof(MPI_Request));*/
                /*out_idx++;*/
            /*}*/
        /*}*/
    /*}*/

    /*// always have the receive port open, for each neighbor of each node*/
    /*// the message sent will be their new estimate for distance to the dest*/
    /*for (int v = 0; v < nodes_per_proc; v++) {*/
        /*for (int i = 0; i < n_out_neighbors[v]; i++) {*/
            /*int n = out_neighbors[v][i];*/
            /*int node = (n / nodes_per_proc);*/
            /*MPI_Request req = irecv_reqs[v][i];*/
            /*// n is the neighbor*/
            /*// i is the index in out_neighbors*/

            /*// alright the tag is gonna be given by*/
            /*// v * n_nodes + n*/
            /*int tag = v * n_nodes + n;*/
            /*// updates from neighbor n to vertex v will go into downstream_updates[v][n];*/
            /*MPI_Irecv(&(downstream_updates[v][i]), 1, MPI_INT, node, tag, MPI_COMM_WORLD, &req);*/
        /*}*/
    /*}*/

    /*// now we can proceed*/
    /*int static_iters = 0;*/
    /*// while (static_iters < ITERATIONS_TO_CONVERGE) {*/
    /*for (int n_iters = 0; n_iters < n_nodes; n_iters++) {*/
        /*// check for any updates from downstream and*/
        /*// use them to recompute local values*/
        /*for (int v = 0; v < nodes_per_proc; v++) {*/
            /*for (int i = 0; i < n_out_neighbors[v]; i++) {*/
                /*int n = out_neighbors[v][i];*/
                /*int proc = (n / nodes_per_proc);*/
                /*// get the MPI_Request*/
                /*MPI_Request req = irecv_reqs[v][i];*/

                /*int flag;*/
                /*MPI_Status status;*/
                /*MPI_Test(&req, &flag, &status);*/

                /*if (flag) {  // then we have an update*/

                    /*// check the value of the downstream updates*/
                    /*int new_est = downstream_updates[v][i];*/
                    /*pprintf("Node %d got update from proc %d node %d (new_est %d)\n", v + offset, proc, n, new_est);*/
                    /*int edge_weight = flat_matrix_get(adj_matrix, v + offset, n);*/
                    /*if (new_est != INT_MAX && new_est + edge_weight < distances[v]) {*/
                        /*distances[v] = new_est + edge_weight;*/
                        /*next_hops[v] = n;*/
                        /*has_local_update = 1;*/
                    /*}*/

                    /*// and now we reopen the irecv*/

                    /*memset(&req, 0, sizeof(MPI_Request));*/
                    /*int tag = v * n_nodes + n;*/
                    /*MPI_Irecv(&(downstream_updates[v][i]), 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &req);*/
                /*}*/

            /*}*/

            /*if (has_local_update) {*/
                /*static_iters = 0;*/
                /*for (int i = 0; i < n_in_neighbors[v]; i++) {*/
                    /*MPI_Request req = isend_reqs[v][i];*/
                    /*int n = in_neighbors[v][i];*/
                    /*int proc = (n / nodes_per_proc);*/
                    /*int tag = v * n_nodes + n;*/
                    /*pprintf("Node %d sending updated estimate %d to proc %d\n", v + offset, distances[v], proc);*/

                    /*int flag;*/
                    /*MPI_Status status;*/
                    /*MPI_Request_get_status(req, &flag, &status);*/
                    /*if (!flag) pprintf("Past send not complete!\n");*/

                    /*MPI_Isend(&distances[v], 1, MPI_INT, proc, tag, MPI_COMM_WORLD, &req);*/
                /*}*/
                /*has_local_update = 0;*/
            /*} else {*/
                /*static_iters++;*/
            /*}*/

        /*}*/

        /*usleep(100000);*/
        /*MPI_Barrier(MPI_COMM_WORLD);*/
    /*}*/

    // then we put a barrier here to make sure everyone returns at the same time
    /*MPI_Barrier(MPI_COMM_WORLD);*/

    //////////////////////////////////////////////////////////////
    // CLEANUP
    //////////////////////////////////////////////////////////////

    for (int i = 0; i < nodes_per_proc; i++) {
        free(in_neighbors[i]);
        free(out_neighbors[i]);
        free(downstream_updates[i]);
    }
    free(in_neighbors);
    free(out_neighbors);
    free(downstream_updates);
    free(n_in_neighbors);
    free(n_out_neighbors);

}
