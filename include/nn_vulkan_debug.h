#ifndef NN_BACKEND_VULKAN_DEBUG_H
#define NN_BACKEND_VULKAN_DEBUG_H

#include <vulkan/vulkan.h>

VkResult nnCreateVkDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);

#endif // NN_BACKEND_VULKAN_DEBUG_H
