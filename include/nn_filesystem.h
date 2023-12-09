#ifndef NN_BACKEND_NN_FILESYSTEM_H
#define NN_BACKEND_NN_FILESYSTEM_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

#include "nn_backend_macros.h"
#include "nn_types.h"

NN_BACKEND_DLL_EXPORT uint8_t *nnReadBinaryFile(const char *path, uint64_t *read_count);

NN_BACKEND_DLL_EXPORT NnStatus nnWriteBinaryFile(const char *path, const uint8_t *data, size_t size);

#endif // NN_BACKEND_NN_FILESYSTEM_H
