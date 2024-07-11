#include "uka-buffer.hpp"
#include <cassert>
#include <cstring>

namespace uka{
    auto Uka_Buffer::map(VkDeviceSize size, VkDeviceSize offset)->VkResult
    {
        return vkMapMemory(device, memory, offset, size, 0, &mapped);
    }

    auto Uka_Buffer::unmap()->void
    {
        if (mapped)
        {
            vkUnmapMemory(device, memory);
            mapped = nullptr;
        }
    }

    auto Uka_Buffer::bind(VkDeviceSize offset)->VkResult
    {
        return vkBindBufferMemory(device, buffer, memory, offset);
    }

    auto Uka_Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)->void
    {
        descriptor.offset = offset;
        descriptor.buffer = buffer;
        descriptor.range = size;
    }

    auto Uka_Buffer::copyTo(void* data, VkDeviceSize size)->void
    {
        assert(mapped);
        memcpy(mapped, data, size);
    }

    auto Uka_Buffer::flush(VkDeviceSize size, VkDeviceSize offset)->VkResult
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
    }

    auto Uka_Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)->VkResult
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
    }

    auto Uka_Buffer::destroy()->void
    {
        if (buffer)
        {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        if (memory)
        {
            vkFreeMemory(device, memory, nullptr);
        }
    }

    auto Uka_Buffer::getDescriptor()->VkDescriptorBufferInfo
    {
        return descriptor;
    }

    auto Uka_Buffer::get()->VkBuffer
    {
        return buffer;
    }
}//namespace uka

