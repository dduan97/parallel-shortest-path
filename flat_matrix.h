#ifndef __FLAT_MATRIX_H__
#define __FLAT_MATRIX_H__

#include <stdlib.h>
#include <stdio.h>
#define WEIGHT int

typedef struct {
    WEIGHT *arr;
    int width;
    int height;
} FlatMatrix;


FlatMatrix *flat_matrix_init(int width, int height);
int flat_matrix_set(FlatMatrix *fm, int r, int c, WEIGHT val);
WEIGHT flat_matrix_get(FlatMatrix *fm, int r, int c);
FlatMatrix *flat_matrix_from_2d_arr(WEIGHT **arr, int width, int height);

void flat_matrix_free(FlatMatrix *fm);

void flat_matrix_print(FlatMatrix *fm);

#endif
