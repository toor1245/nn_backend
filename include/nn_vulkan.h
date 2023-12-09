#ifndef NN_BACKEND_LIBRARY_H
#define NN_BACKEND_LIBRARY_H

#include "nn_backend_macros.h"
#include "nn_types.h"
#include <vulkan/vulkan.h>

NN_BACKEND_DLL_EXPORT VkResult nnCreateDefaultVkInstance(VkInstance *instance);

NN_BACKEND_DLL_EXPORT VkPhysicalDevice nnGetVkPhysicalDeviceIndexByExtensionName(VkPhysicalDevice *physical_devices,
                                                                                 uint32_t num_physical_devices,
                                                                                 const char *ext_name);

NN_BACKEND_DLL_EXPORT VkPhysicalDevice *nnGetVkPhysicalDevices(VkInstance instance, uint32_t *physical_devices_count);

NN_BACKEND_DLL_EXPORT int nnGetVkQueueComputeIndex(VkPhysicalDevice physical_device);

NN_BACKEND_DLL_EXPORT NnMemoryProps nnGetMemoryProperties(VkPhysicalDevice physical_device);

NN_BACKEND_DLL_EXPORT VkDevice nnCreateVkDevice(VkPhysicalDevice physical_device, uint32_t queue_compute_index);

NN_BACKEND_DLL_EXPORT VkQueue nnGetVkDeviceQueue(VkDevice device, uint32_t queue_compute_index);

NN_BACKEND_DLL_EXPORT int32_t nnFindMemoryProperties(NnMemoryProps *pMemoryProperties,
                                                     uint32_t memoryTypeBitsRequirement,
                                                     VkMemoryPropertyFlags requiredProperties);

#endif // NN_BACKEND_LIBRARY_H
