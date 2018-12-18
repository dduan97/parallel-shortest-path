#include "flat_matrix.h"

FlatMatrix *flat_matrix_init(int width, int height) {
    FlatMatrix *fm = malloc(sizeof(FlatMatrix));
    fm->width = width;
    fm->height = height;
    fm->arr = calloc(width * height, sizeof(WEIGHT));
    return fm;
}

int flat_matrix_set(FlatMatrix *fm, int r, int c, WEIGHT val) {
    if (c >= fm->height || c >= fm->width) {
        return -1;
    }
    // the index might be bigger than an int
    unsigned long idx = r * fm->width + c;
    fm->arr[idx] = val;
}

WEIGHT flat_matrix_get(FlatMatrix *fm, int r, int c) {
    if (r >= fm->height || c >= fm->width) {
        return -1;
    }
    unsigned long idx = r * fm->width + c;
    return fm->arr[idx];
}


FlatMatrix *flat_matrix_from_2d_arr(WEIGHT **arr, int width, int height) {
    FlatMatrix *fm = malloc(sizeof(FlatMatrix));
    fm->width = width;
    fm->height = height;
    fm->arr = malloc(width * height * sizeof(WEIGHT));
    // now we fill in the matrix
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            flat_matrix_set(fm, r, c, arr[r][c]);
        }
    }
    return fm;
}

void flat_matrix_free(FlatMatrix *fm) {
    free(fm->arr);
    free(fm);
}

void flat_matrix_print(FlatMatrix *fm) {
    for (int r = 0; r < fm->height; r++) {
        for (int c = 0; c < fm->width; c++) {
            printf("%3d ", flat_matrix_get(fm, r, c));
        }
        printf("\n");
    }
}
