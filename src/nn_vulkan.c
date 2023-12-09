#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "nn_vulkan.h"
#include "nn_utils.h"
#include "nn_types.h"

static const char *enabledInstanceExtensions[] = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
};

static bool nnExistsValidationLayer() {
    VkResult result;

    uint32_t layerCount;
    result = vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    NN_CHECK_RESULT(result);

    VkLayerProperties *availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    NN_CHECK_RESULT(result);

    for (int i = 0; i < 1; ++i) {
        const char *layerName = validationLayers[i];

        for (int j = 0; j < layerCount; ++j) {
            if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                return true;
            }
        }
    }
    return false;
}

VkResult nnCreateDefaultVkInstance(VkInstance *instance) {
    VkApplicationInfo applicationInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "CNN library",
            1,
            "none",
            0,
            VK_MAKE_VERSION(1, 3, 0),
    };

    VkValidationFeaturesEXT validationFeatures = {};
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.enabledValidationFeatureCount = 1;

    VkValidationFeatureEnableEXT enabledValidationFeatures[1] = {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

    VkInstanceCreateInfo instanceCreateInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &applicationInfo,
            NN_ARRAY_LENGTH(validationLayers),
            validationLayers,
            NN_ARRAY_LENGTH(enabledInstanceExtensions),
            enabledInstanceExtensions,
    };

    validationFeatures.pNext = instanceCreateInfo.pNext;
    instanceCreateInfo.pNext = &validationFeatures;

    if (!nnExistsValidationLayer()) {
        return VK_ERROR_UNKNOWN;
    }

    VkResult result;
    result = vkCreateInstance(&instanceCreateInfo, NULL, instance);
    return result;
}

VkPhysicalDevice *vkGetPhysicalDevices(VkInstance instance, uint32_t *physical_devices_count) {
    uint32_t num_physical_devices;
    NN_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, NULL));

    *physical_devices_count = num_physical_devices;
    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
    NN_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
    return physical_devices;
}

VkPhysicalDevice
nnGetVkPhysicalDeviceIndexByExtensionName(VkPhysicalDevice *physical_devices, uint32_t num_physical_devices,
                                          const char *ext_name) {
    VkResult result;
    int device_index = -1;
    uint32_t num_extensions;
    VkExtensionProperties *extensions = NULL;

    for (int i = 0; i < num_physical_devices; ++i) {
        num_extensions = 0;

        result = vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_extensions, NULL);
        NN_CHECK_RESULT(result);

        extensions = (VkExtensionProperties *) malloc(sizeof(VkExtensionProperties) * num_extensions);
        result = vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_extensions, extensions);
        NN_CHECK_RESULT(result);

        for (int j = 0; j < num_extensions; ++j) {
            NN_PRINTF("i: %d, extension: %s\n", i, extensions[j].extensionName);
            if (strcmp(extensions[j].extensionName, ext_name) == 0) {
                device_index = i;
                break;
            }
        }
        free(extensions);
    }

    if (device_index == -1) {
        NN_PRINTF("Physical Device is not found can't use nn_backend");
        exit(EXIT_FAILURE);
    }
    return physical_devices[device_index];
}

VkPhysicalDevice *nnGetVkPhysicalDevices(VkInstance instance, uint32_t *physical_devices_count) {
    uint32_t num_physical_devices;
    NN_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, NULL));

    *physical_devices_count = num_physical_devices;
    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
    NN_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
    return physical_devices;
}

int nnGetVkQueueComputeIndex(VkPhysicalDevice physical_device) {
    uint32_t num_queue_families = 0;
    int queue_compute_index = -1;
    VkQueueFamilyProperties *queue_family_properties = NULL;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, NULL);

    queue_family_properties = malloc(sizeof(VkQueueFamilyProperties) * num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, &queue_family_properties[0]);

    for (int i = 0; i < num_queue_families; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queue_compute_index = i;
            break;
        }
    }

    free(queue_family_properties);

    return queue_compute_index;
}

NnMemoryProps nnGetMemoryProperties(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties memory_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_props);

    NnMemoryProps props;
    props.memory_type_count = memory_props.memoryTypeCount;

    // TODO: create cleanup function to free memory.
    props.memory_types = malloc(sizeof(NnMemoryType) * NN_MAX_MEMORY_TYPES);
    memcpy(props.memory_types, memory_props.memoryTypes, NN_MAX_MEMORY_TYPES);

    return props;
}

int32_t nnFindMemoryProperties(NnMemoryProps *memory_props,
                               uint32_t memory_type_bits_requirement,
                               VkMemoryPropertyFlags required_props) {
    const uint32_t memory_count = memory_props->memory_type_count;

    for (uint32_t memory_index = 0; memory_index < memory_count; ++memory_index) {
        const uint32_t memory_type_bits = (1 << memory_index);
        const bool is_required_memory_type = memory_type_bits_requirement & memory_type_bits;

        const VkMemoryPropertyFlags properties = memory_props->memory_types[memory_index].property_flags;
        const bool has_required_props = (properties & required_props) == required_props;

        if (is_required_memory_type && has_required_props) {
            return (int32_t) (memory_index);
        }
    }

    return -1;
}

static const char *enabledDeviceExtensions[] = {
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
        VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
};

VkDevice nnCreateVkDevice(VkPhysicalDevice physical_device, uint32_t queue_compute_index) {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,
            0,
            (uint32_t) queue_compute_index,
            1,
            &queuePriority,
    };

    VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            NULL,
            0,
            1,
            &deviceQueueCreateInfo,
            0,
            NULL,
            NN_ARRAY_LENGTH(enabledDeviceExtensions),
            enabledDeviceExtensions,
            NULL,
    };

    VkDevice device;
    VkResult result = vkCreateDevice(physical_device, &deviceCreateInfo, NULL, &device);
    NN_CHECK_RESULT(result);
    return device;
}

VkQueue nnGetVkDeviceQueue(VkDevice device, uint32_t queue_compute_index) {
    VkQueue queue;
    vkGetDeviceQueue(device, queue_compute_index, 0u, &queue);
    return queue;
}
