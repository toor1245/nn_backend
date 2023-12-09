#ifndef NN_BACKEND_NN_VULKAN_PIPELINE_H
#define NN_BACKEND_NN_VULKAN_PIPELINE_H

#include "nn_backend_macros.h"
#include <vulkan/vulkan.h>

NN_BACKEND_DLL_EXPORT VkPipelineCache nnCreateVkPipelineCache(VkDevice device, const char *path);

NN_BACKEND_DLL_EXPORT VkDescriptorSetLayoutBinding nnCreateVkDescriptorSetLayoutBindingStorageBuffer(int binding);

NN_BACKEND_DLL_EXPORT VkPipeline nnCreateVkComputePipeline2MatricesAndOutput(VkDevice device,
                                                                             VkPipelineCache pipeline_cache,
                                                                             const char *shader_path);

NN_BACKEND_DLL_EXPORT void nnSaveVkPipelineCache(VkDevice device, VkPipelineCache pipeline_cache);

NN_BACKEND_DLL_EXPORT VkPipeline nnCreateVkComputePipelineCorrelate2dValid(VkDevice device,
                                                                           VkPipelineCache pipeline_cache);

#endif // NN_BACKEND_NN_VULKAN_PIPELINE_H
