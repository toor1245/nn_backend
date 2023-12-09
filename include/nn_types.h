#ifndef NN_BACKEND_NN_TYPES_H
#define NN_BACKEND_NN_TYPES_H

#include <vulkan/vulkan.h>

#define NN_MAX_MEMORY_TYPES VK_MAX_MEMORY_TYPES

typedef struct {
    uint32_t property_flags;
    uint32_t heap_index;
} NnMemoryType;

typedef struct {
    uint32_t memory_type_count;
    NnMemoryType* memory_types;
} NnMemoryProps;

typedef enum {
    NN_SUCCESS = 0,
    NN_FAILURE = 1
} NnStatus;

typedef struct {
    VkDevice device;
    uint32_t queue_compute_index;
    NnMemoryProps memory_props;
    VkQueue queue;
    VkPipeline pipeline_cache;
} NnComputeInfo;

typedef struct {
    float* ptr;
    uint32_t rows;
    uint32_t columns;
} NnMatrix;

#endif // NN_BACKEND_NN_TYPES_H
