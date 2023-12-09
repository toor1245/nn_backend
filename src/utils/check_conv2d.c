#include <malloc.h>
#include <memory.h>
#include "nn_compute.h"
#include "nn_utils.h"

int main() {

    const int input_m = 3;
    const int input_n = 3;
    float *input_ptr = malloc((input_m * input_n) * sizeof(float));

    input_ptr[0 * input_n + 0] = 1;
    input_ptr[0 * input_n + 1] = 6;
    input_ptr[0 * input_n + 2] = 2;

    input_ptr[1 * input_n + 0] = 5;
    input_ptr[1 * input_n + 1] = 3;
    input_ptr[1 * input_n + 2] = 1;

    input_ptr[2 * input_n + 0] = 7;
    input_ptr[2 * input_n + 1] = 0;
    input_ptr[2 * input_n + 2] = 4;

    NnMatrix input = {
            .rows = input_m,
            .columns = input_n,
            .ptr = input_ptr
    };

    const int kernel_m = 2;
    const int kernel_n = 2;
    float *kernel_ptr = malloc((kernel_m * kernel_n) * sizeof(float));

    NnMatrix kernel = {
            .rows = kernel_m,
            .columns = kernel_n,
            .ptr = kernel_ptr
    };

    kernel_ptr[0 * kernel_n + 0] = 1;
    kernel_ptr[0 * kernel_n + 1] = 2;

    kernel_ptr[1 * kernel_n + 0] = -1;
    kernel_ptr[1 * kernel_n + 1] = 0;

    const int output_m = 4;
    const int output_n = 4;
    float *output_ptr = malloc((output_m * output_n) * sizeof(float));
    memset(output_ptr, 0, (output_m * output_n) * sizeof(float));

    NnMatrix output = {
            .rows = output_m,
            .columns = output_n,
            .ptr = output_ptr
    };

    nnFullCrossCorrelationCpu(&input, &kernel, &output);

    NnMatrix* output_ref = &output;
    NN_PRINT_MATRIX(output_ref);
}