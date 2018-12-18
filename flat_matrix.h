#include "helpers.h"

typedef struct {
    WEIGHT *arr;
    size_t width;
    size_t height;
} FlatMatrix;

void flat_matrix_set(FlatMatrix *fm, size_t r, size_t c, WEIGHT val);
WEIGHT flat_matrix_get(FlatMatrix *fm, size_t r, size_t c);
FlatMatrix *flat_matrix_from_2d_arr(WEIGHT **arr, size_t width, size_t height);
