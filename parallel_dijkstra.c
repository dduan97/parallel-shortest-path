#include <stdio.h>
#include <limits.h>
#include <mpi.h>

#include "helpers.h"
#include "benchmarks.h"

int parallel_dijkstra(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors);

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

    WEIGHT **adj_matrix = NULL;
    int rank, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    // the master proc will generate the graph
    if (rank == 0) {
        adj_matrix = gen_graph(n_nodes, n_edges, max_weight);
        if (adj_matrix == NULL) {
            exit(1);
        }
    }

    // master proc should split the graph up and distribute to each node
    // also each proc should have their own version of distances[] and predecessors[]

    // now we get our cluster.
    // ASSUME that num_procs | n_nodes
    int nodes_per_proc = n_nodes / n_procs;
    int cluster_start = rank * nodes_per_proc;

    // now to distribute the

    // function signatures should look like
    // int shortest_path(adj_matrix, n_nodes, source, WEIGHT *distances, int **paths)
    // distances and paths will be the outputs
    //  distances[i] will be the cost of the shortest path going from source to i
    //  paths[i] will be the actual path. Each row in paths will have length n_nodes and will
    //      essentially be "null-terminated" with the element i. If distances[i] is infinity, then
    //      paths[i] is invalid.

    //print_array(adj_matrix, n_nodes);

    size_t *dijkstra_predecessors = calloc(nodes_per_proc, sizeof(size_t));

    WEIGHT *dijkstra_distances = calloc(nodes_per_proc, sizeof(WEIGHT));


    timing(&start_wall, &cpu);
    parallel_dijkstra(adj_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    dijkstra_distances,
                    dijkstra_predecessors);

    timing(&end_wall, &cpu);
    printf("Dijkstra's time: %.4f\n", end_wall - start_wall);

    WEIGHT *serial_distances = calloc(n_nodes, sizeof(WEIGHT));

    timing(&start_wall, &cpu);
    serial_dijkstra(adj_matrix,
                    n_nodes,
                    n_edges,
                    0,
                    serial_distances,
                    serial_predecessors);

    timing(&end_wall, &cpu);
    printf("BF's time: %.4f\n", end_wall - start_wall);

    // use the serial version to verify results

    double l2 = l2_norm(dijkstra_distances, serial_distances, n_nodes);
    printf("\n\nL2 Norm: %lf\n", l2);

    ///////////////////////////////////////////////////////////////////////////
    ///  CLEAN UP
    ///////////////////////////////////////////////////////////////////////////
    free(dijkstra_predecessors);
    free(serial_predecessors);
    free(dijkstra_distances);
    free(serial_distances);
    free_array(adj_matrix, n_nodes);

    return 0;
}

int parallel_dijkstra(WEIGHT **adj_matrix, size_t n_nodes, size_t n_edges, size_t src, WEIGHT *distances, size_t *predecessors) {
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
    int cluster_start = rank * nodes_per_proc;

    // each node should have

    MQNode **mqns = malloc(nodes_per_proc * sizeof(MQNode)); // oh boy
    if (src - cluster_start > 0 && src - cluster_start < nodes_per_proc) {
    }

}
