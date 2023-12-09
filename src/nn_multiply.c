#include "nn_compute.h"

void nnMultiply(NnMatrix *ma, NnMatrix *mb, NnMatrix *mc) {
    float *ptr_ma = ma->ptr;
    float *ptr_mb = mb->ptr;
    float *ptr_mc = mc->ptr;

    for (int i = 0; i < ma->rows; i++) {
        float *c = ptr_mc + i * mb->columns;

        for (int k = 0; k < ma->columns; k++) {
            float *b = ptr_mb + k * mb->columns;
            float a = ptr_ma[i * ma->columns + k];

            for (int j = 0; j < mb->columns; j++) {
                c[j] = c[j] + a * b[j];
            }
        }
    }
}
