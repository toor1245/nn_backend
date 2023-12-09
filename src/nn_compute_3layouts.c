#include <vulkan/vulkan.h>
#include <memory.h>

#include "nn_compute.h"
#include "nn_utils.h"
#include "nn_types.h"
#include "nn_vulkan.h"
#include "nn_vulkan_pipeline.h"

void nnRunTwoMatricesAndOutput(NnComputeInfo *info, NnMatrix *matrixA, NnMatrix *matrixB, NnMatrix *matrixC) {
    const uint32_t compute_index = info->queue_compute_index;
    const uint32_t layoutBindingsCount = 3;
    const uint32_t elements = matrixA->rows * matrixA->columns;

    NN_PRINT_MATRIX(matrixA);
    NN_PRINT_MATRIX(matrixB);
    NN_PRINT_MATRIX(matrixC);

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
            .size = sizeof(float) * elements,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = (const uint32_t *) &compute_index
    };

    VkBuffer bufferA;
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &bufferA));

    VkBuffer bufferB;
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &bufferB));

    VkBuffer bufferC;
    NN_CHECK_RESULT(vkCreateBuffer(info->device, &bufferCreateInfo, NULL, &bufferC));

    VkDeviceSize requiredMemorySize = 0;
    VkMemoryRequirements bufferAMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, bufferA, &bufferAMemoryRequirements);
    requiredMemorySize += bufferAMemoryRequirements.size;
    VkMemoryRequirements bufferBMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, bufferA, &bufferBMemoryRequirements);
    requiredMemorySize += bufferBMemoryRequirements.size;
    VkMemoryRequirements bufferResultMemoryRequirements;
    vkGetBufferMemoryRequirements(info->device, bufferA,
                                  &bufferResultMemoryRequirements);
    requiredMemorySize += bufferResultMemoryRequirements.size;

    int memory_type_index = nnFindMemoryProperties(&info->memory_props, bufferAMemoryRequirements.memoryTypeBits,
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
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, bufferA, memory, memoryOffset));

    memoryOffset += bufferAMemoryRequirements.size;
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, bufferB, memory, memoryOffset));

    memoryOffset += bufferBMemoryRequirements.size;
    NN_CHECK_RESULT(vkBindBufferMemory(info->device, bufferC, memory, memoryOffset));

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
    VkDescriptorBufferInfo bufferAInfo = {
            .buffer = bufferA,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.pBufferInfo = &bufferAInfo;
    descriptorSetWrites[0] = writeDescriptorSet;


    VkDescriptorBufferInfo bufferBInfo = {
            .buffer = bufferB,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.pBufferInfo = &bufferBInfo;
    descriptorSetWrites[1] = writeDescriptorSet;


    VkDescriptorBufferInfo bufferCInfo = {
            .buffer = bufferC,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };

    writeDescriptorSet.dstBinding = 2;
    writeDescriptorSet.pBufferInfo = &bufferCInfo;
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

    vkCmdDispatch(commandBuffer, elements, 1, 1);

    NN_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    float *data = NULL;
    NN_CHECK_RESULT(vkMapMemory(info->device, memory, 0, VK_WHOLE_SIZE, 0, (void **) &data));

    float *aData = data;
    float *bData = data + elements;
    float *resultData = bData + elements;

    memcpy(aData, matrixA->ptr, elements * sizeof(float));
    memcpy(bData, matrixB->ptr, elements * sizeof(float));

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

    memcpy(matrixC->ptr, resultData, elements * sizeof(float));

    NN_PRINT_MATRIX(matrixC);

    vkUnmapMemory(info->device, memory);
    free(layoutBindings);
    free(descriptorSetWrites);
}
