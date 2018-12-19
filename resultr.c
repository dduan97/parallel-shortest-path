#include "resultr.h"

// don't think we need more than that
int KEY_LEN = 64;
const char *dist_fmt = "%d ";
const char *pred_fmt = "%d ";

static int get_filename(char *buf, int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo) {
    return sprintf(buf, "./results/%d_%d_%d_%d_%d.results", n_nodes, n_edges, max_weight, seed, algo);
}

static int get_matrix_filename(char *buf, int seed, int n_nodes, int n_edges, int max_weight) {
    return sprintf(buf, "./results/%d_%d_%d_%d.matrix", n_nodes, n_edges, max_weight, seed);
}

static int matrix_to_file(char *filename, FlatMatrix *fm) {
    FILE *fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("Could not open file %s\n", filename);
        return -1;
    }
    for (int r = 0; r < fm->height; r++) {
        for (int c = 0; c < fm->width; c++) {
            fprintf(fp, "%3d ", flat_matrix_get(fm, r, c));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 0;
}

static int results_to_file(char *filename, int n_nodes, WEIGHT *distances, int *predecessors) {
    FILE *fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("Could not open file %s\n", filename);
        return -1;
    }
    // write the weights in one line
    for (int i = 0; i < n_nodes; i++) {
        fprintf(fp, dist_fmt, distances[i]);
    }
    fprintf(fp, "\n");
    // now the predecessors
    for (int i = 0; i < n_nodes; i++) {
        fprintf(fp, pred_fmt, predecessors[i]);
    }
    fclose(fp);
    return 0;
}

// does not override the existing result file
int store_result_soft(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors) {
    char buf[KEY_LEN];
    if (get_filename(buf, seed, n_nodes, n_edges, max_weight, algo) == -1) {
        printf("Error generating filename!\n");
        return -1;
    }

    // check if file exists (i know there can be race conditions like this but
    // don't think thats a big deal here
    struct stat fstat;
    if (stat(buf, &fstat) == -1) {
        // then the file doesn't exist and we are free to write to it
        return results_to_file(buf, n_nodes, distances, predecessors);
    } else {
        printf("File %s already exists!\n", buf);
        return -1;
    }
}


int store_result_hard(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors) {
    char buf[KEY_LEN];
    if (get_filename(buf, seed, n_nodes, n_edges, max_weight, algo) == -1) {
        printf("ERROR generating filename\n");
        return -1;
    }
    return results_to_file(buf, n_nodes, distances, predecessors);
}

// here, distances and predecessors will be output buffers
int read_result(int seed, int n_nodes, int n_edges, int max_weight, ALGORITHM algo, WEIGHT *distances, int *predecessors) {
    char buf[KEY_LEN];
    if (get_filename(buf, seed, n_nodes, n_edges, max_weight, algo) == -1) {
        printf("ERROR generating filename\n");
        return -1;
    }
    FILE *fp = fopen(buf, "r");
    if (fp == NULL) {
        printf("Could not open file %s\n", buf);
        return -1;
    }
    // now we read
    // first the distances
    for (int i = 0; i < n_nodes; i++) {
        fscanf(fp, dist_fmt, distances + i);
    }
    // now the newline
    fscanf(fp, "\n");
    // now the predecessors
    for (int i = 0; i < n_nodes; i++) {
        fscanf(fp, pred_fmt, predecessors + i);
    }
    return 1;
}

// does not override the existing result file
int store_matrix_soft(int seed, int n_nodes, int n_edges, int max_weight, FlatMatrix *fm) {
    char buf[KEY_LEN];
    if (get_matrix_filename(buf, seed, n_nodes, n_edges, max_weight) == -1) {
        printf("Error generating filename!\n");
        return -1;
    }

    // check if file exists (i know there can be race conditions like this but
    // don't think thats a big deal here
    struct stat fstat;
    if (stat(buf, &fstat) == -1) {
        // then the file doesn't exist and we are free to write to it
        return matrix_to_file(buf, fm);
    } else {
        printf("File %s already exists!\n", buf);
        return -1;
    }
}
