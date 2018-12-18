#ifndef __FLAT_MATRIX_H__
#define __FLAT_MATRIX_H__

#include <stdlib.h>
#include <stdio.h>
#define WEIGHT int

typedef struct {
    WEIGHT *arr;
    size_t width;
    size_t height;
} FlatMatrix;


FlatMatrix *flat_matrix_init(size_t width, size_t height);
int flat_matrix_set(FlatMatrix *fm, size_t r, size_t c, WEIGHT val);
WEIGHT flat_matrix_get(FlatMatrix *fm, size_t r, size_t c);
FlatMatrix *flat_matrix_from_2d_arr(WEIGHT **arr, size_t width, size_t height);

void flat_matrix_free(FlatMatrix *fm);

void flat_matrix_print(FlatMatrix *fm);

#endif
