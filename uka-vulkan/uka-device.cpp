#include "uka-device.hpp"

namespace uka
{
    Uka_Device::Uka_Device(VkPhysicalDevice physical_device)
    {
        assert(physical_device);
        this->physical_device = physical_device;

        vkGetPhysicalDeviceProperties(physical_device, &properties);
        vkGetPhysicalDeviceFeatures(physical_device, &features);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        assert(queue_family_count > 0);
        queue_family_properties.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
        if (extension_count > 0)
        {
            std::vector<VkExtensionProperties> extensions(extension_count);
            if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data()) == VK_SUCCESS)
            {
                for (auto &extension : extensions)
                {
                    supported_extensions.push_back(extension.extensionName);
                }
            }
        }
    }
    Uka_Device::~Uka_Device()
    {
        if (command_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(logical_device, command_pool, nullptr);
        }
        if (logical_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(logical_device, nullptr);
        }
    }
    auto Uka_Device::get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties, uint32_t *type_index)->uint32_t
    {
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        {
            if ((type_bits & 1) == 1)
            {
                if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    *type_index = i;
                    return i;
                }
            }
            type_bits >>= 1;
        }
        throw std::runtime_error("Could not find a suitable memory type!");
    }
    auto Uka_Device::get_queue_family_index(VkQueueFlagBits queue_flags)->uint32_t
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
        {
            if ((queue_family_properties[i].queueFlags & queue_flags) == queue_flags)
            {
                return i;
            }
        }
        throw std::runtime_error("Could not find a matching queue family index");
    }

    auto Uka_Device::create_logical_device(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain, bool useSwapChain, VkQueueFlags requestedQueueTypes)->VkResult
    {
        auto queue_infos = std::vector<VkDeviceQueueCreateInfo>{};
        if(requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
        {
            queue_family_indices.graphics = get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
            VkDeviceQueueCreateInfo queue_info{};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue_family_indices.graphics;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = new float{1.0f};
            queue_infos.push_back(queue_info);
        }
        else
        {
            queue_family_indices.graphics = 0;
        }

        if(requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
        {
            queue_family_indices.compute = get_queue_family_index(VK_QUEUE_COMPUTE_BIT);
            if(queue_family_indices.compute != queue_family_indices.graphics)
            {
                VkDeviceQueueCreateInfo queue_info{};
                queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_info.queueFamilyIndex = queue_family_indices.compute;
                queue_info.queueCount = 1;
                queue_info.pQueuePriorities = new float{1.0f};
                queue_infos.push_back(queue_info);
            }
        }
        else
        {
            queue_family_indices.compute = queue_family_indices.graphics;
        }

        if(requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
        {
            queue_family_indices.transfer = get_queue_family_index(VK_QUEUE_TRANSFER_BIT);
            if(queue_family_indices.transfer != queue_family_indices.graphics && queue_family_indices.transfer != queue_family_indices.compute)
            {
                VkDeviceQueueCreateInfo queue_info{};
                queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_info.queueFamilyIndex = queue_family_indices.transfer;
                queue_info.queueCount = 1;
                queue_info.pQueuePriorities = new float{1.0f};
                queue_infos.push_back(queue_info);
            }
        }
        else
        {
            queue_family_indices.transfer = queue_family_indices.graphics;
        }

        auto device_extensions = std::vector<const char*>{};
        if(useSwapChain)
        {
            device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        auto device_create_info = VkDeviceCreateInfo{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
        device_create_info.pQueueCreateInfos = queue_infos.data();
        device_create_info.pEnabledFeatures = &enabledFeatures;

        auto physical_device_feature2 = VkPhysicalDeviceFeatures2{};
        if(pNextChain)
        {
            physical_device_feature2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physical_device_feature2.features = enabledFeatures;
            physical_device_feature2.pNext = pNextChain;
            device_create_info.pEnabledFeatures = nullptr;
            device_create_info.pNext = &physical_device_feature2;
        }

        if(device_extensions.size() > 0)
        {
            for(auto extension : device_extensions)
            {
                if(!extension_supported(extension))
                {
                    throw std::runtime_error("Extension not supported");
                }
            }
            device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
            device_create_info.ppEnabledExtensionNames = device_extensions.data();
        }

        this->enabled_features = enabledFeatures;

        auto result = vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device);
        if(result != VK_SUCCESS)
        {
            return result;
        }

        command_pool = create_command_pool(queue_family_indices.graphics);
        return result;

    }
    auto Uka_Device::create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)->VkResult
    {
        auto buffer_info = uka::init::buffer_create_info(usage,size);
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_info, nullptr, buffer));

        auto mem_requirements = VkMemoryRequirements{};
        auto memory_allocate_info = uka::init::memory_allocate_info();
        vkGetBufferMemoryRequirements(logical_device, *buffer, &mem_requirements);
        memory_allocate_info.allocationSize = mem_requirements.size;
        memory_allocate_info.memoryTypeIndex = get_memory_type(mem_requirements.memoryTypeBits, memory_properties);

        auto allocal_flag = VkMemoryAllocateFlagsInfo{};

        if(usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocal_flag.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            allocal_flag.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            memory_allocate_info.pNext = &allocal_flag;
        }
        VK_CHECK_RESULT(vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, memory));

        if(data)
        {
            void* mapped;
            VK_CHECK_RESULT(vkMapMemory(logical_device, *memory, 0, size, 0, &mapped));
            memcpy(mapped, data, size);
            if(!(memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                VkMappedMemoryRange mapped_range = uka::init::mapped_memory_range();
                mapped_range.memory = *memory;
                mapped_range.offset = 0;
                mapped_range.size = size;
                vkFlushMappedMemoryRanges(logical_device, 1, &mapped_range);
            }
            vkUnmapMemory(logical_device, *memory);
        }

        VK_CHECK_RESULT(vkBindBufferMemory(logical_device, *buffer, *memory, 0));

        return VK_SUCCESS;
    }

    auto Uka_Device::create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties, VkDeviceSize size, Uka_Buffer* buffer, VkDeviceMemory *memory, Uka_Buffer *data)->VkResult
    {
        buffer->device = logical_device;

        auto buffer_info = uka::init::buffer_create_info(usage,size);
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_info, nullptr,&buffer->buffer));

        auto mem_requirements = VkMemoryRequirements{};
        auto memory_allocate_info = uka::init::memory_allocate_info();
        vkGetBufferMemoryRequirements(logical_device, buffer->buffer, &mem_requirements);
        memory_allocate_info.allocationSize = mem_requirements.size;
        memory_allocate_info.memoryTypeIndex = get_memory_type(mem_requirements.memoryTypeBits, memory_properties);

        auto allocal_flag = VkMemoryAllocateFlagsInfo{};

        if(usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocal_flag.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            allocal_flag.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            memory_allocate_info.pNext = &allocal_flag;
        }
        VK_CHECK_RESULT(vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, memory));

        buffer->alignment = mem_requirements.alignment;
		buffer->size = size;
		buffer->usageFlags = usage;
		buffer->memoryPropertyFlags = memory_properties;

        if(data)
        {
            VK_CHECK_RESULT(buffer->map());
            memcpy(buffer->mapped, data->mapped, size);
            if(!(memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                VK_CHECK_RESULT(buffer->flush());
            }
            buffer->unmap();
        }

        buffer->setupDescriptor();

        return VK_SUCCESS;
    }

    auto Uka_Device::copy_buffer(Uka_Buffer* src, Uka_Buffer* dst, VkQueue queue, VkBufferCopy *copy_region)->void
    {
        assert(src->size >= dst->size);
        assert(src->buffer);
        auto command_buffer = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        auto buffer_copy = VkBufferCopy{};
        if(copy_region == nullptr)
        {
            buffer_copy.size = src->size;
        }
        else{
            buffer_copy = *copy_region;

        }
        vkCmdCopyBuffer(command_buffer, src->buffer, dst->buffer, 1, &buffer_copy);
        flush_command_buffer(command_buffer, queue);
    }

    auto Uka_Device::create_command_pool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)->VkCommandPool
    {
        auto command_pool_info = VkCommandPoolCreateInfo{};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.queueFamilyIndex = queueFamilyIndex;
        command_pool_info.flags = createFlags;
        auto command_pool = VkCommandPool{};
        VK_CHECK_RESULT(vkCreateCommandPool(logical_device, &command_pool_info, nullptr, &command_pool));
        return command_pool;
    }

    auto Uka_Device::create_command_buffer(VkCommandBufferLevel level,
        VkCommandPool pool,
        bool begin) -> VkCommandBuffer
    {
        auto command_buffer_info = uka::init::command_buffer_allocate_info(command_pool, level, 1);
        auto command_buffer = VkCommandBuffer{};
        VK_CHECK_RESULT(vkAllocateCommandBuffers(logical_device, &command_buffer_info, &command_buffer));
        if(begin)
        {
            auto begin_info = uka::init::command_buffer_begin_info();
            VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &begin_info));
        }
        return command_buffer;
    }

    auto Uka_Device::create_command_buffer(VkCommandBufferLevel level, bool begin)->VkCommandBuffer
    {
        return create_command_buffer(level, command_pool, begin);
    }

    auto Uka_Device::flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)->void
    {
        if(commandBuffer == VK_NULL_HANDLE)
        {
            return;
        }
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
        auto submit_info = uka::init::submit_info();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &commandBuffer;
        auto fence_info = uka::init::fence_create_info();
        auto fence = VkFence{};
        VK_CHECK_RESULT(vkCreateFence(logical_device, &fence_info, nullptr, &fence));
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence));
        VK_CHECK_RESULT(vkWaitForFences(logical_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
        vkDestroyFence(logical_device, fence, nullptr);
        if(free)
        {
            vkFreeCommandBuffers(logical_device, command_pool, 1, &commandBuffer);
        }

    }

    auto Uka_Device::flush_command_buffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)->void
    {
        flush_command_buffer(commandBuffer, queue, free);
    }

    auto Uka_Device::extension_supported(const char *extension)->bool
    {
        return std::find(supported_extensions.begin(), supported_extensions.end(), extension) != supported_extensions.end();
    }

    auto Uka_Device::get_support_depth_format(bool check_sampling_support)->VkFormat
    {
        auto depth_formats = std::vector<VkFormat>{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
        for(auto& format : depth_formats)
        {
            auto format_properties = VkFormatProperties{};
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
            if(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                if(check_sampling_support)
                {
                    if(!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
                    {
                        continue;
                    }
                }
                return format;
            }
        }
        throw std::runtime_error("Could not find a supported depth format");
    }
}
