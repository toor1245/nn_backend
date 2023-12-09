#include "string.h"
#include "nn_memory.h"

void nnMemoryCopy(void *dest, const void *src, const size_t size) {
    memcpy(dest, src, size);
}
