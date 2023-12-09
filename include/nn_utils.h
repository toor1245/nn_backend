#ifndef NN_BACKEND_NN_UTILS_H
#define NN_BACKEND_NN_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#define NN_ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

#define _NN_CHECK_RESULT(r) do { \
    if ((r) != VK_SUCCESS) { \
        printf("result = %d, line = %d\n", (r), __LINE__);  \
        exit(0);  \
    }   \
} while (0)

#if NN_BACKEND_VALIDATION_LAYER
#define NN_CHECK_RESULT(r) _NN_CHECK_RESULT(r)
#else
#define NN_CHECK_RESULT(r) _NN_CHECK_RESULT(r)
#endif

#if NN_BACKEND_VALIDATION_LAYER
#define NN_PRINTF(fmt, ...)                             \
    do {                                                \
        fprintf(stderr, fmt, ##__VA_ARGS__);            \
    } while (0)
#else
#define NN_PRINTF(fmt, ...) do { } while (0)
#endif

#if NN_BACKEND_VALIDATION_LAYER
#define NN_PRINT_MATRIX(m)                                       \
    do {                                                         \
        NN_PRINTF("rows: %d, columns: %d", m->rows, m->columns); \
        NN_PRINTF("\n");                                         \
        for (int i = 0; i < m->rows; ++i) {                      \
            for (int j = 0; j < m->columns; ++j) {               \
                NN_PRINTF("%f ", m->ptr[i * m->columns + j]);    \
            }                                                    \
            NN_PRINTF("\n");                                     \
        }                                                        \
        NN_PRINTF("\n");                                         \
    } while(0)
#else
#define NN_PRINT_MATRIX(matrix) do { } while(0)
#endif

#endif // NN_BACKEND_NN_UTILS_H
