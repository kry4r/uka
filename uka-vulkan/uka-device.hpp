#pragma once

#include <vulkan/vulkan.h>
#include "uka-buffer.hpp"
#include "uka-tools.hpp"
#include <algorithm>
#include <cassert>
#include <exception>


namespace uka{
    struct Uka_Device
    {
        VkPhysicalDevice physical_device;
        VkDevice logical_device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceFeatures enabled_features;
        VkPhysicalDeviceMemoryProperties memory_properties;
        std::vector<VkQueueFamilyProperties> queue_family_properties;
        std::vector<std::string> supported_extensions;
        VkCommandPool command_pool = VK_NULL_HANDLE;

        struct
        {
            uint32_t graphics;
            uint32_t compute;
            uint32_t transfer;
        } queue_family_indices;

        operator VkDevice() const
        {
            return logical_device;
        }

        explicit Uka_Device(VkPhysicalDevice physical_device);
        ~Uka_Device();
        auto get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties, uint32_t *type_index = nullptr)->uint32_t;
        auto get_queue_family_index(VkQueueFlagBits queue_flags)->uint32_t;
        auto create_logical_device(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)->VkResult;
        auto create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr)->VkResult;
        auto create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkDeviceSize size, Uka_Buffer* buffer, VkDeviceMemory *memory, Uka_Buffer *data)->VkResult;
        auto copy_buffer(Uka_Buffer* src, Uka_Buffer* dst, VkQueue queue, VkBufferCopy *copy_region)->void;
        auto create_command_pool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)->VkCommandPool;
        auto create_command_buffer(VkCommandBufferLevel level, bool begin = false)->VkCommandBuffer;
        auto create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false)->VkCommandBuffer;
        auto flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true)->void;
        auto flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true)->void;
        auto extension_supported(const char *extension)->bool;
        auto get_support_depth_format(bool check_sampling_support)->VkFormat;
    };
}
