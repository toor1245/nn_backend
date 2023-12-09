#ifndef NN_BACKEND_NN_COPY_H
#define NN_BACKEND_NN_COPY_H

#include <stdint.h>
#include "nn_backend_macros.h"

NN_BACKEND_DLL_EXPORT void nnMemoryCopy(void *dest, const void *src, size_t size);

#endif // NN_BACKEND_NN_COPY_H
