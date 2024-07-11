#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>


namespace uka{
    struct Uka_Buffer
    {
        VkDevice device;
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorBufferInfo descriptor;
        VkDeviceSize size = 0;
        VkDeviceSize alignment = 0;
        void* mapped = nullptr;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;

        auto map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)->VkResult;
        auto unmap()->void;
        auto bind(VkDeviceSize offset = 0)->VkResult;
        auto setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)->void;
        auto copyTo(void* data, VkDeviceSize size)->void;
        auto flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)->VkResult;
        auto invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)->VkResult;
        auto destroy()->void;

        auto getDescriptor()->VkDescriptorBufferInfo;
        auto get()->VkBuffer;
    };
}//namespace uka
