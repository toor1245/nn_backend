#include "nn_vulkan_pipeline.h"
#include "nn_filesystem.h"
#include "nn_utils.h"

VkDescriptorSetLayoutBinding nnCreateVkDescriptorSetLayoutBindingStorageBuffer(int binding) {
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    return layoutBinding;
}

VkPipelineCache nnCreateVkPipelineCache(VkDevice device, const char *path) {
    uint64_t read_count;
    uint8_t *pipeline_data;

    pipeline_data = nnReadBinaryFile(path, &read_count);

    VkPipelineCacheCreateInfo cache_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext = NULL,
            .initialDataSize = read_count,
            .pInitialData = pipeline_data,
    };
    VkPipelineCache pipeline_cache;
    NN_CHECK_RESULT(vkCreatePipelineCache(device, &cache_info, NULL, &pipeline_cache));

    if (pipeline_data != NULL) {
        free(pipeline_data);
    }

    return pipeline_cache;
}

VkPipeline nnCreateVkComputePipeline2MatricesAndOutput(VkDevice device, VkPipelineCache pipeline_cache, const char* shader_path) {
    uint32_t layoutBindingsCount = 3;
    VkDescriptorSetLayoutBinding *layoutBindings = (VkDescriptorSetLayoutBinding *) malloc(
            sizeof(VkDescriptorSetLayoutBinding) * layoutBindingsCount);

    for (int i = 0; i < layoutBindingsCount; ++i) {
        layoutBindings[i] = nnCreateVkDescriptorSetLayoutBindingStorageBuffer(i);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            NULL,
            0,
            layoutBindingsCount,
            layoutBindings,
    };

    VkDescriptorSetLayout descriptorSetLayout;
    NN_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            1,
            &descriptorSetLayout,
            0,
            NULL
    };

    VkPipelineLayout pipelineLayout;
    NN_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    uint64_t code_size;
    uint8_t *shader_ptr = nnReadBinaryFile(shader_path, &code_size);

    if (shader_ptr == NULL) {
        NN_PRINTF("[FATAL]: Failed to load shader.");
        exit(EXIT_FAILURE);
    }

    VkShaderModuleCreateInfo shader_module_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .pCode = (const uint32_t *) shader_ptr,
            .codeSize = code_size,
    };

    VkShaderModule shaderModule;
    NN_CHECK_RESULT(vkCreateShaderModule(device, &shader_module_create_info, NULL, &shaderModule));

    VkComputePipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage.stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .stage.module = shaderModule,
            .stage.pName = "main",
            .layout = pipelineLayout
    };

    VkPipeline pipeline;
    NN_CHECK_RESULT(vkCreateComputePipelines(device, pipeline_cache, 1,
                                             &pipelineCreateInfo, NULL, &pipeline));

    free(shader_ptr);
    free(layoutBindings);
    vkDestroyShaderModule(device, shaderModule, NULL);
    return pipeline;
}

VkPipeline nnCreateVkComputePipelineCorrelate2dValid(VkDevice device, VkPipelineCache pipeline_cache) {
    uint32_t layoutBindingsCount = 3;
    VkDescriptorSetLayoutBinding *layoutBindings = (VkDescriptorSetLayoutBinding *) malloc(
            sizeof(VkDescriptorSetLayoutBinding) * layoutBindingsCount);

    for (int i = 0; i < layoutBindingsCount; ++i) {
        layoutBindings[i] = nnCreateVkDescriptorSetLayoutBindingStorageBuffer(i);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            NULL,
            0,
            layoutBindingsCount,
            layoutBindings,
    };

    VkDescriptorSetLayout descriptorSetLayout;
    NN_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            1,
            &descriptorSetLayout,
            0,
            NULL
    };

    VkPipelineLayout pipelineLayout;
    NN_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    uint64_t code_size;
    uint8_t *shader_ptr = nnReadBinaryFile("shaders/correlate2d_valid2.spv", &code_size);

    if (shader_ptr == NULL) {
        NN_PRINTF("[FATAL]: Failed to load shader.");
        exit(EXIT_FAILURE);
    }

    VkShaderModuleCreateInfo shader_module_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .pCode = (const uint32_t *) shader_ptr,
            .codeSize = code_size,
    };

    VkShaderModule shaderModule;
    NN_CHECK_RESULT(vkCreateShaderModule(device, &shader_module_create_info, NULL, &shaderModule));

    VkComputePipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage.stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .stage.module = shaderModule,
            .stage.pName = "main",
            .layout = pipelineLayout
    };

    VkPipeline pipeline;
    NN_CHECK_RESULT(vkCreateComputePipelines(device, pipeline_cache, 1,
                                             &pipelineCreateInfo, NULL, &pipeline));

    free(shader_ptr);
    free(layoutBindings);
    vkDestroyShaderModule(device, shaderModule, NULL);
    return pipeline;
}

void nnSaveVkPipelineCache(VkDevice device, VkPipelineCache pipeline_cache) {
    size_t size;
    NN_CHECK_RESULT(vkGetPipelineCacheData(device, pipeline_cache, &size, NULL));

    uint8_t *data = (uint8_t * )malloc(size);
    NN_CHECK_RESULT(vkGetPipelineCacheData(device, pipeline_cache, &size, data));

    NN_CHECK_RESULT(nnWriteBinaryFile("pipeline_caches/pipeline_cache.data", data, size));
    vkDestroyPipelineCache(device, pipeline_cache, NULL);
    free(data);
}
