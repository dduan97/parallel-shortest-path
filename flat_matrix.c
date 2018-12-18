#include "flat_matrix.h"

FlatMatrix *flat_matrix_init(size_t width, size_t height) {
    FlatMatrix *fm = malloc(sizeof(FlatMatrix));
    fm->width = width;
    fm->height = height;
    fm->arr = malloc(width * height * sizeof(WEIGHT));
    return fm;
}

void flat_matrix_set(FlatMatrix *fm, size_t r, size_t c, WEIGHT val) {
    size_t idx = r * fm->width + c;
    fm->arr[idx] = val;
}

WEIGHT flat_matrix_get(FlatMatrix *fm, size_t r, size_t c) {
    size_t idx = r * fm->width + c;
    return fm->arr[idx];
}


FlatMatrix *flat_matrix_from_2d_arr(WEIGHT **arr, size_t width, size_t height) {
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
