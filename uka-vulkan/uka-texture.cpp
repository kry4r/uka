#include "uka-texture.hpp"


namespace uka
{
    auto Uka_Texture::update_descriptor() ->void
    {
        descriptor.sampler = sampler;
        descriptor.imageView = image_view;
        descriptor.imageLayout = image_layout;
    }

    auto Uka_Texture::destroy()->void
    {
        vkDestroyImageView(this->device->logical_device, image_view, nullptr);
        vkDestroyImage(this->device->logical_device, image, nullptr);
        if(sampler)
            vkDestroySampler(this->device->logical_device, sampler, nullptr);
        vkFreeMemory(this->device->logical_device, device_memory, nullptr);
    }

    auto Uka_Texture::load_ktx(std::string file_path, ktxTexture** ktx_texture) ->ktxResult
    {
        auto result = KTX_SUCCESS;
        //if(!uka::tools::file_exists(file_path))
        //{
        //    uka::tools::exit_fatal("Could not load texture from " + file_path + " file not found.",-1);
        //}
        result = ktxTexture_CreateFromNamedFile(file_path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, ktx_texture);

        return result;
    }

    auto Uka_Texture2D::load(std::string file_path,
                    VkFormat format, Uka_Device* device,
                    VkQueue copy_queue,
                    VkImageUsageFlags img_usage,
                    VkImageLayout img_layout,
                    bool force_linear) -> void
    {
        ktxTexture* ktx_texture;
        auto result = load_ktx(file_path, &ktx_texture);
        assert(result == KTX_SUCCESS);

        this->device = device;
        width = ktx_texture->baseWidth;
        height = ktx_texture->baseHeight;
        mip_levels = ktx_texture->numLevels;

        auto* ktx_texture_data = ktxTexture_GetData(ktx_texture);
        auto ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

        auto format_properties  = VkFormatProperties();
        vkGetPhysicalDeviceFormatProperties(this->device->physical_device, format, &format_properties);

        auto use_staging = !force_linear;

        auto mem_alloc_info = VkMemoryAllocateInfo();
        auto mem_reqs = VkMemoryRequirements();

        auto command_buffer = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        if(use_staging)
        {
            auto staging_buffer = VkBuffer();
            auto staging_memory = VkDeviceMemory();

            auto buffer_create_info = uka::init::buffer_create_info();
            buffer_create_info.size = ktx_texture_size;
            buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VK_CHECK_RESULT(vkCreateBuffer(this->device->logical_device, &buffer_create_info, nullptr, &staging_buffer));

            vkGetBufferMemoryRequirements(this->device->logical_device, staging_buffer, &mem_reqs);

            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &staging_memory));
            VK_CHECK_RESULT(vkBindBufferMemory(this->device->logical_device, staging_buffer, staging_memory, 0));

            uint8_t* data;
            VK_CHECK_RESULT(vkMapMemory(this->device->logical_device, staging_memory, 0, mem_alloc_info.allocationSize, 0, (void**)&data));
            memcpy(data, ktx_texture_data, ktx_texture_size);
            vkUnmapMemory(this->device->logical_device, staging_memory);

            auto buffer_copy_region = std::vector<VkBufferImageCopy>();

            for(auto i = 0; i < mip_levels; i++)
            {
                auto offset = ktx_size_t();
                auto result = ktxTexture_GetImageOffset(ktx_texture, i,0,0, &offset);
                assert(result == KTX_SUCCESS);

                auto buffer_image_copy = VkBufferImageCopy();
                buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.mipLevel = i;
                buffer_image_copy.imageSubresource.baseArrayLayer = 0;
                buffer_image_copy.imageSubresource.layerCount = 1;
                buffer_image_copy.imageExtent.width = std::max(1u, width >> i);
                buffer_image_copy.imageExtent.height = std::max(1u, height >> i);
                buffer_image_copy.imageExtent.depth = 1;
                buffer_image_copy.bufferOffset = offset;
                buffer_copy_region.push_back(buffer_image_copy);
            }

            auto image_create_info = uka::init::image_create_info();
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = format;
            image_create_info.extent = {width, height, 1};
            image_create_info.mipLevels = mip_levels;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = img_usage;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if(!(image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
            {
                image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            VK_CHECK_RESULT(vkCreateImage(this->device->logical_device, &image_create_info, nullptr, &image));
            vkGetImageMemoryRequirements(this->device->logical_device, image, &mem_reqs);
            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &device_memory));
            VK_CHECK_RESULT(vkBindImageMemory(this->device->logical_device, image, device_memory, 0));

            auto image_subresource_range = VkImageSubresourceRange();
            image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_subresource_range.baseMipLevel = 0;
            image_subresource_range.levelCount = mip_levels;
            image_subresource_range.layerCount = 1;

            uka::tools::set_image_layout(command_buffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_subresource_range);
            vkCmdCopyBufferToImage(command_buffer, staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_region.size(), buffer_copy_region.data());

            this->image_layout = img_layout;
            uka::tools::set_image_layout(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, img_layout, image_subresource_range);

            this->device->flush_command_buffer(command_buffer, copy_queue);

            vkDestroyBuffer(this->device->logical_device, staging_buffer, nullptr);
            vkFreeMemory(this->device->logical_device, staging_memory, nullptr);

        }
        else
        {
            assert(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

            auto mappable_image = VkImage();
            auto mappable_memory = VkDeviceMemory();

            auto image_create_info = uka::init::image_create_info();
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = format;
            image_create_info.extent = {width, height, 1};
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
            image_create_info.usage = img_usage;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VK_CHECK_RESULT(vkCreateImage(this->device->logical_device, &image_create_info, nullptr, &mappable_image));

            vkGetImageMemoryRequirements(this->device->logical_device, mappable_image, &mem_reqs);
            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &mappable_memory));
            VK_CHECK_RESULT(vkBindImageMemory(this->device->logical_device, mappable_image, mappable_memory, 0));

            auto image_subresource = VkImageSubresource();
            image_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_subresource.mipLevel = 0;

            auto image_subresource_layout = VkSubresourceLayout();
            vkGetImageSubresourceLayout(this->device->logical_device, mappable_image, &image_subresource, &image_subresource_layout);
            void* data;

            VK_CHECK_RESULT(vkMapMemory(this->device->logical_device, mappable_memory, 0, mem_alloc_info.allocationSize, 0, &data));

            memcpy(data, ktx_texture_data, ktx_texture_size);
            vkUnmapMemory(this->device->logical_device, mappable_memory);

            this->image = mappable_image;
            this->device_memory = mappable_memory;
            this->image_layout = img_layout;

            uka::tools::set_image_layout(command_buffer, image, VK_IMAGE_LAYOUT_UNDEFINED, img_layout, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
            this->device->flush_command_buffer(command_buffer, copy_queue);

        }

        ktxTexture_Destroy(ktx_texture);

        auto sample_create_info = VkSamplerCreateInfo();
        sample_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sample_create_info.magFilter = VK_FILTER_LINEAR;
        sample_create_info.minFilter = VK_FILTER_LINEAR;
        sample_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sample_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sample_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sample_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sample_create_info.mipLodBias = 0.0f;
        sample_create_info.compareOp = VK_COMPARE_OP_NEVER;
        sample_create_info.minLod = 0.0f;
        sample_create_info.maxLod = use_staging ? (float)mip_levels : 0.0f;
        sample_create_info.maxAnisotropy = this->device->enabled_features.samplerAnisotropy ? this->device->properties.limits.maxSamplerAnisotropy : 1.0f;
        sample_create_info.anisotropyEnable = this->device->enabled_features.samplerAnisotropy;
        sample_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(this->device->logical_device, &sample_create_info, nullptr, &sampler));

        auto image_view_create_info = VkImageViewCreateInfo();
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_create_info.subresourceRange.levelCount = use_staging ? mip_levels : 1;
        image_view_create_info.image = image;
        VK_CHECK_RESULT(vkCreateImageView(this->device->logical_device, &image_view_create_info, nullptr, &image_view));

        update_descriptor();


    }

    auto Uka_Texture2D_Array::load(std::string file_path,
        VkFormat format,
        uka::Uka_Device* device,
        VkQueue copy_queue,
        VkImageUsageFlags img_usage,
        VkImageLayout img_layout) -> void
    {
        ktxTexture* ktx_texture;
        auto result = load_ktx(file_path, &ktx_texture);
        assert(result == KTX_SUCCESS);

        this->device = device;
        width = ktx_texture->baseWidth;
        height = ktx_texture->baseHeight;
        mip_levels = ktx_texture->numLevels;
        layer_count = ktx_texture->numLayers;

        auto* ktx_texture_data = ktxTexture_GetData(ktx_texture);
        auto ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

        auto mem_alloc_info = uka::init::memory_allocate_info();
        auto mem_reqs = VkMemoryRequirements();

        auto stage_buffer = VkBuffer();
        auto stage_memory = VkDeviceMemory();

        auto buffer_create_info = uka::init::buffer_create_info();
        buffer_create_info.size = ktx_texture_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(this->device->logical_device, &buffer_create_info, nullptr, &stage_buffer));

        vkGetBufferMemoryRequirements(this->device->logical_device, stage_buffer, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &stage_memory));
        VK_CHECK_RESULT(vkBindBufferMemory(this->device->logical_device, stage_buffer, stage_memory, 0));

        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(this->device->logical_device, stage_memory, 0, mem_alloc_info.allocationSize, 0, (void**)&data));
        memcpy(data, ktx_texture_data, ktx_texture_size);
        vkUnmapMemory(this->device->logical_device, stage_memory);

        auto buffer_copy_region = std::vector<VkBufferImageCopy>();
        for(auto layer = 0;layer<layer_count;layer++)
        {
            for(auto i = 0;i<mip_levels;i++)
            {
                auto offset = ktx_size_t();
                auto result = ktxTexture_GetImageOffset(ktx_texture, i, 0, layer, &offset);
                assert(result == KTX_SUCCESS);

                auto buffer_image_copy = VkBufferImageCopy();
                buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.mipLevel = i;
                buffer_image_copy.imageSubresource.baseArrayLayer = layer;
                buffer_image_copy.imageSubresource.layerCount = 1;
                buffer_image_copy.imageExtent.width = std::max(1u, width >> i);
                buffer_image_copy.imageExtent.height = std::max(1u, height >> i);
                buffer_image_copy.imageExtent.depth = 1;
                buffer_image_copy.bufferOffset = offset;
                buffer_copy_region.push_back(buffer_image_copy);
            }
        }

        auto image_create_info = uka::init::image_create_info();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.extent = {width, height, 1};
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = img_usage;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if(!(image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
        {
            image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        image_create_info.arrayLayers = layer_count;
        image_create_info.mipLevels = mip_levels;
        VK_CHECK_RESULT(vkCreateImage(this->device->logical_device, &image_create_info, nullptr, &image));

        vkGetImageMemoryRequirements(this->device->logical_device, image, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &device_memory));
        VK_CHECK_RESULT(vkBindImageMemory(this->device->logical_device, image, device_memory, 0));

        auto copy_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        auto image_subresource_range = VkImageSubresourceRange();
        image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_subresource_range.baseMipLevel = 0;
        image_subresource_range.levelCount = mip_levels;
        image_subresource_range.layerCount = layer_count;

        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_subresource_range);
        vkCmdCopyBufferToImage(copy_cmd, stage_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_region.size(), buffer_copy_region.data());
        this->image_layout = img_layout;
        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, img_layout, image_subresource_range);
        device->flush_command_buffer(copy_cmd, copy_queue);

        auto sampler_create_info = VkSamplerCreateInfo();
        sampler_create_info.magFilter = VK_FILTER_LINEAR;
        sampler_create_info.minFilter = VK_FILTER_LINEAR;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeV = sampler_create_info.addressModeU;
        sampler_create_info.addressModeW = sampler_create_info.addressModeU;
        sampler_create_info.mipLodBias = 0.0f;
        sampler_create_info.maxAnisotropy = device->enabled_features.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
        sampler_create_info.anisotropyEnable = device->enabled_features.samplerAnisotropy;
        sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_create_info.minLod = 0.0f;
        sampler_create_info.maxLod = (float)mip_levels;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        VK_CHECK_RESULT(vkCreateSampler(device->logical_device, &sampler_create_info, nullptr, &sampler));

        auto image_view_create_info = VkImageViewCreateInfo();
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_create_info.subresourceRange.levelCount = mip_levels;
        image_view_create_info.subresourceRange.layerCount = layer_count;
        image_view_create_info.image = image;
        VK_CHECK_RESULT(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));

        ktxTexture_Destroy(ktx_texture);
        vkDestroyBuffer(device->logical_device, stage_buffer, nullptr);
        vkFreeMemory(device->logical_device, stage_memory, nullptr);

        update_descriptor();

    }

    auto Uka_Texture_Cube::load(std::string file_path,
        VkFormat format,
        uka::Uka_Device* device,
        VkQueue copy_queue,
        VkImageUsageFlags img_usage,
        VkImageLayout img_layout) -> void
    {
        ktxTexture* ktx_texture;
        auto result = load_ktx(file_path, &ktx_texture);
        assert(result == KTX_SUCCESS);

        this->device = device;
        width = ktx_texture->baseWidth;
        height = ktx_texture->baseHeight;
        mip_levels = ktx_texture->numLevels;

        auto* ktx_texture_data = ktxTexture_GetData(ktx_texture);
        auto ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

        auto mem_alloc_info = uka::init::memory_allocate_info();
        auto mem_reqs = VkMemoryRequirements();

        auto stage_buffer = VkBuffer();
        auto stage_memory = VkDeviceMemory();

        auto buffer_create_info = uka::init::buffer_create_info();
        buffer_create_info.size = ktx_texture_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(this->device->logical_device, &buffer_create_info, nullptr, &stage_buffer));

        vkGetBufferMemoryRequirements(this->device->logical_device, stage_buffer, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &stage_memory));
        VK_CHECK_RESULT(vkBindBufferMemory(this->device->logical_device, stage_buffer, stage_memory, 0));

        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(this->device->logical_device, stage_memory, 0, mem_alloc_info.allocationSize, 0, (void**)&data));
        memcpy(data, ktx_texture_data, ktx_texture_size);
        vkUnmapMemory(this->device->logical_device, stage_memory);

        auto buffer_copy_region = std::vector<VkBufferImageCopy>();
        for(auto face = 0;face<6;face++)
        {
            for(auto i = 0;i<mip_levels;i++)
            {
                auto offset = ktx_size_t();
                auto result = ktxTexture_GetImageOffset(ktx_texture, i, 0, face, &offset);
                assert(result == KTX_SUCCESS);

                auto buffer_image_copy = VkBufferImageCopy();
                buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.mipLevel = i;
                buffer_image_copy.imageSubresource.baseArrayLayer = face;
                buffer_image_copy.imageSubresource.layerCount = 1;
                buffer_image_copy.imageExtent.width = std::max(1u, width >> i);
                buffer_image_copy.imageExtent.height = std::max(1u, height >> i);
                buffer_image_copy.imageExtent.depth = 1;
                buffer_image_copy.bufferOffset = offset;
                buffer_copy_region.push_back(buffer_image_copy);
            }
        }

        auto image_create_info = uka::init::image_create_info();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.extent = {width, height, 1};
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = img_usage;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if(!(image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
        {
            image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        image_create_info.arrayLayers = 6;
        image_create_info.mipLevels = mip_levels;
        image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        VK_CHECK_RESULT(vkCreateImage(this->device->logical_device, &image_create_info, nullptr, &image));

        vkGetImageMemoryRequirements(this->device->logical_device, image, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(this->device->logical_device, &mem_alloc_info, nullptr, &device_memory));
        VK_CHECK_RESULT(vkBindImageMemory(this->device->logical_device, image, device_memory, 0));

        auto copy_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        auto image_subresource_range = VkImageSubresourceRange();
        image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_subresource_range.baseMipLevel = 0;
        image_subresource_range.levelCount = mip_levels;
        image_subresource_range.layerCount = 6;

        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_subresource_range);
        vkCmdCopyBufferToImage(copy_cmd, stage_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_region.size(), buffer_copy_region.data());
        this->image_layout = img_layout;
        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, img_layout, image_subresource_range);
        device->flush_command_buffer(copy_cmd, copy_queue);

        auto sampler_create_info = VkSamplerCreateInfo();
        sampler_create_info.magFilter = VK_FILTER_LINEAR;
        sampler_create_info.minFilter = VK_FILTER_LINEAR;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeV = sampler_create_info.addressModeU;
        sampler_create_info.addressModeW = sampler_create_info.addressModeU;
        sampler_create_info.mipLodBias = 0.0f;
        sampler_create_info.maxAnisotropy = device->enabled_features.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
        sampler_create_info.anisotropyEnable = device->enabled_features.samplerAnisotropy;
        sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_create_info.minLod = 0.0f;
        sampler_create_info.maxLod = (float)mip_levels;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        VK_CHECK_RESULT(vkCreateSampler(device->logical_device, &sampler_create_info, nullptr, &sampler));

        auto image_view_create_info = VkImageViewCreateInfo();
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_create_info.subresourceRange.levelCount = mip_levels;
        image_view_create_info.subresourceRange.layerCount = 6;
        image_view_create_info.image = image;
        VK_CHECK_RESULT(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));

        ktxTexture_Destroy(ktx_texture);
        vkDestroyBuffer(device->logical_device, stage_buffer, nullptr);
        vkFreeMemory(device->logical_device, stage_memory, nullptr);

        update_descriptor();
    }

}
