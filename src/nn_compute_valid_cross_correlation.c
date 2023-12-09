#include <stdio.h>
#include <memory.h>
#include "nn_vulkan.h"
#include "nn_utils.h"
#include "nn_vulkan_pipeline.h"
#include "nn_compute.h"

void nnFullCrossCorrelationCpu(NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output) {
    NN_PRINT_MATRIX(matrix);
    NN_PRINT_MATRIX(kernel);
    NN_PRINT_MATRIX(output);

    int i, j, m, n;

    for (i = 0; i < output->rows; ++i) {
        for (j = 0; j < output->columns; ++j) {
            for (m = 0; m < kernel->rows; ++m) {
                for (n = 0; n < kernel->columns; ++n) {
                    int rowIndex = i - m;
                    int colIndex = j - n;
                    if (rowIndex >= 0 && rowIndex < matrix->rows && colIndex >= 0 && colIndex < matrix->columns) {
                        output->ptr[i * output->columns + j] +=
                                matrix->ptr[rowIndex * matrix->columns + colIndex] * kernel->ptr[m * kernel->columns + n];
                    }
                }
            }
        }
    }
}

void nnValidCrossCorrelationCpu(NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output) {
    NN_PRINT_MATRIX(matrix);
    NN_PRINT_MATRIX(kernel);
    NN_PRINT_MATRIX(output);

    for (int i = 0; i < output->rows; ++i) {
        for (int j = 0; j < output->columns; ++j) {
            for (int m = 0; m < kernel->rows; ++m) {
                for (int n = 0; n < kernel->columns; ++n) {
                    output->ptr[i * output->columns + j] +=
                            matrix->ptr[(i + m) * matrix->columns + (j + n)] * kernel->ptr[m * kernel->columns + n];
                }
            }
        }
    }
}

void nnValidCrossCorrelationGpu(NnComputeInfo *info, NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output) {
    const uint32_t compute_index = info->queue_compute_index;
    const uint32_t layoutBindingsCount = 3;
    const int matrixLength = (int) (matrix->rows * matrix->columns);
    const int kernelLength = (int) (kernel->rows * kernel->columns);
    const int outputLength = (int) (output->rows * output->columns);

    NN_PRINT_MATRIX(kernel);
    NN_PRINTF("%d\n", kernelLength);

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
    NN_CHECK_RESULT(
            vkCreateDescriptorSetLayout(info->device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));

    VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(float) * (matrix->rows * matrix->columns),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = (const uint32_t *) &compute_index
    };

    VkBuffer inputBuffer;
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &inputBuffer));

    VkBuffer kernelBuffer;
    bufferCreateInfo.size = sizeof(float) * (kernel->rows * kernel->columns);
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &kernelBuffer));

    VkBuffer outputBuffer;
    bufferCreateInfo.size = sizeof(float) * (output->rows * output->columns);
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &outputBuffer));

    VkDeviceSize requiredMemorySize = 0;

    VkMemoryRequirements inputBufferMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, inputBuffer, &inputBufferMemoryRequirements);
    requiredMemorySize += inputBufferMemoryRequirements.size;

    VkMemoryRequirements kernelBufferMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, kernelBuffer, &kernelBufferMemoryRequirements);
    requiredMemorySize += kernelBufferMemoryRequirements.size;

    VkMemoryRequirements outputBufferMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, outputBuffer,
                                  &outputBufferMemoryRequirements);
    requiredMemorySize += outputBufferMemoryRequirements.size;

    int memory_type_index = nnFindMemoryProperties(&info->memory_props, inputBufferMemoryRequirements.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = requiredMemorySize,
            .memoryTypeIndex = memory_type_index,
    };
    VkDeviceMemory memory;
    NN_CHECK_RESULT(vkAllocateMemory(info->device, &allocateInfo, NULL, &memory));

    VkDeviceSize memoryOffset = 0;
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, inputBuffer, memory, memoryOffset));

    memoryOffset += inputBufferMemoryRequirements.size;
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, kernelBuffer, memory, memoryOffset));

    memoryOffset += kernelBufferMemoryRequirements.size;
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, outputBuffer, memory, memoryOffset));

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
    NN_CHECK_RESULT(vkCreatePipelineLayout(info->device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

    VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = layoutBindingsCount
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
    };
    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(info->device, &descriptorPoolCreateInfo, NULL, &descriptorPool);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
    };

    VkDescriptorSet descriptorSet;
    NN_CHECK_RESULT(vkAllocateDescriptorSets(info->device, &descriptorSetAllocateInfo, &descriptorSet));

    VkWriteDescriptorSet *descriptorSetWrites = malloc(sizeof(VkWriteDescriptorSet) * 3);
    VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };
    VkDescriptorBufferInfo inputBufferInfo = {
            .buffer = inputBuffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.pBufferInfo = &inputBufferInfo;
    descriptorSetWrites[0] = writeDescriptorSet;


    VkDescriptorBufferInfo kernelBufferInfo = {
            .buffer = kernelBuffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.pBufferInfo = &kernelBufferInfo;
    descriptorSetWrites[1] = writeDescriptorSet;


    VkDescriptorBufferInfo outputBufferInfo = {
            .buffer = outputBuffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };

    writeDescriptorSet.dstBinding = 2;
    writeDescriptorSet.pBufferInfo = &outputBufferInfo;
    descriptorSetWrites[2] = writeDescriptorSet;

    vkUpdateDescriptorSets(info->device, 3, descriptorSetWrites, 0, NULL);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = compute_index
    };
    VkCommandPool commandPool;
    NN_CHECK_RESULT(vkCreateCommandPool(info->device, &commandPoolCreateInfo, NULL, &commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    NN_CHECK_RESULT(vkAllocateCommandBuffers(info->device, &commandBufferAllocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, info->pipeline_cache);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    vkCmdDispatch(commandBuffer, matrix->columns, matrix->rows, 1);

    NN_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    float *data = NULL;
    NN_CHECK_RESULT(vkMapMemory(info->device, memory, 0, VK_WHOLE_SIZE, 0, (void **) &data));

    float *aData = data;
    float *bData = data + matrixLength;
    float *resultData = bData + kernelLength;

    memcpy(aData, matrix->ptr, matrixLength * sizeof(float));
    memcpy(bData, kernel->ptr, kernelLength * sizeof(float));

    vkUnmapMemory(info->device, memory);

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(info->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(info->queue);

    data = NULL;
    NN_CHECK_RESULT(vkMapMemory(info->device, memory, 0, VK_WHOLE_SIZE, 0, (void **) &data));

    memcpy(output->ptr, resultData, outputLength * sizeof(float));

    vkUnmapMemory(info->device, memory);
}
