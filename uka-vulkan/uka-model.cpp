#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "uka-model.hpp"

VkDescriptorSetLayout uka::gltf::descriptor_set_layout_image = VK_NULL_HANDLE;
VkDescriptorSetLayout uka::gltf::descriptor_set_layout_ubo = VK_NULL_HANDLE;
VkMemoryPropertyFlags uka::gltf::memory_property_flags = 0;
uint32_t uka::gltf::descriptor_binding_flags = uka::gltf::descriptor_binding_flags::image_base_color;

auto load_image_data_function(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData) ->bool
{
    if (image->uri.find_last_of(".") != std::string::npos) {
        if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") {
            return true;
        }
    }
    return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

auto load_image_data_function_empty(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData) ->bool
{
    // This function will be used for samples that don't require images to be loaded
    return true;
}

auto uka::gltf::Texture::update_descriptor() -> void
{
    descriptor.sampler = sampler;
    descriptor.imageView = image_view;
    descriptor.imageLayout = image_layout;
}

auto uka::gltf::Texture::destroy() -> void
{
    vkDestroySampler(device->logical_device, sampler, nullptr);
    vkDestroyImageView(device->logical_device, image_view, nullptr);
    vkDestroyImage(device->logical_device, image, nullptr);
    vkFreeMemory(device->logical_device, device_memory, nullptr);
}

auto uka::gltf::Texture::from_gltf_image(const tinygltf::Image& gltf_image,
    std::string path,
    uka::Uka_Device* device,
    VkQueue copy_queue) -> void
{
    this->device = device;
    auto is_ktx = false;
    if (gltf_image.uri.find_last_of(".") != std::string::npos) {
        if (gltf_image.uri.substr(gltf_image.uri.find_last_of(".") + 1) == "ktx") {
            is_ktx = true;
        }
    }

    auto format = VkFormat{};
    if(!is_ktx)
    {
        unsigned char* buffer = nullptr;
        auto buffer_size = 0;
        auto delete_buffer = false;
        if(gltf_image.component == 3)
        {
            buffer_size = gltf_image.width * gltf_image.height * 4;
            buffer = new unsigned char[buffer_size];
            auto rgba = buffer;
            auto rgb = &gltf_image.image[0];
            for (size_t i = 0; i < gltf_image.width * gltf_image.height; ++i) {
                for (int32_t j = 0; j < 3; ++j) {
                    rgba[j] = rgb[j];
                }
                rgba += 4;
                rgb += 3;
            }
            delete_buffer = true;
        }
        else
        {
            buffer = const_cast<unsigned char*>(&gltf_image.image[0]);
            buffer_size = gltf_image.image.size();
        }

        format = VK_FORMAT_R8G8B8A8_UNORM;

        auto format_properties = VkFormatProperties{};
        width = gltf_image.width;
        height = gltf_image.height;
        mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        vkGetPhysicalDeviceFormatProperties(device->physical_device, format, &format_properties);
        assert(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        assert(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

        auto mem_alloc_info = VkMemoryAllocateInfo{};
        mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        auto mem_reqs = VkMemoryRequirements{};

        auto stage_buffer = VkBuffer{};
        auto stage_memory = VkDeviceMemory{};

        auto buffer_info = VkBufferCreateInfo{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = buffer_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device->logical_device, &buffer_info, nullptr, &stage_buffer));
        vkGetBufferMemoryRequirements(device->logical_device, stage_buffer, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logical_device, &mem_alloc_info, nullptr, &stage_memory));
        VK_CHECK_RESULT(vkBindBufferMemory(device->logical_device, stage_buffer, stage_memory, 0));

        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(device->logical_device, stage_memory, 0, mem_reqs.size, 0, (void**)&data));
        memcpy(data, buffer, buffer_size);
        vkUnmapMemory(device->logical_device, stage_memory);

        auto image_info = VkImageCreateInfo{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = format;
        image_info.mipLevels = mip_levels;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.extent = { width, height, 1 };
        image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK_RESULT(vkCreateImage(device->logical_device, &image_info, nullptr, &image));
        vkGetImageMemoryRequirements(device->logical_device, image, &mem_reqs);
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logical_device, &mem_alloc_info, nullptr, &device_memory));
        VK_CHECK_RESULT(vkBindImageMemory(device->logical_device, image, device_memory, 0));

        auto copy_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        auto subresource_range = VkImageSubresourceRange{};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = mip_levels;

        auto image_memory_barrier = VkImageMemoryBarrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.image = image;
        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.subresourceRange = subresource_range;
        vkCmdPipelineBarrier(copy_cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        auto buffer_image_copy = VkBufferImageCopy{};
        buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_image_copy.imageSubresource.mipLevel = 0;
        buffer_image_copy.imageSubresource.baseArrayLayer = 0;
        buffer_image_copy.imageSubresource.layerCount = 1;
        buffer_image_copy.imageExtent = { width, height, 1 };
        vkCmdCopyBufferToImage(copy_cmd, stage_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange = subresource_range;
        vkCmdPipelineBarrier(copy_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        device->flush_command_buffer(copy_cmd, copy_queue);

        vkDestroyBuffer(device->logical_device, stage_buffer, nullptr);
        vkFreeMemory(device->logical_device, stage_memory, nullptr);

        auto blit_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        for(auto i=1;i<mip_levels;i++)
        {
            auto image_blit = VkImageBlit{};
            image_blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_blit.srcSubresource.layerCount = 1;
            image_blit.srcSubresource.mipLevel = i - 1;
            image_blit.srcOffsets[1] = { int32_t(width >> (i - 1)), int32_t(height >> (i - 1)), 1 };
            image_blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_blit.dstSubresource.layerCount = 1;
            image_blit.dstSubresource.mipLevel = i;
            image_blit.dstOffsets[1] = { int32_t(width >> i), int32_t(height >> i), 1 };

            auto mip_subresource_range = VkImageSubresourceRange{};
            mip_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mip_subresource_range.baseMipLevel = i;
            mip_subresource_range.levelCount = 1;
            mip_subresource_range.layerCount = 1;

            auto image_memory_barrier = VkImageMemoryBarrier{};
            image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_memory_barrier.image = image;
            image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.subresourceRange = mip_subresource_range;
            vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

            vkCmdBlitImage(blit_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_blit, VK_FILTER_LINEAR);

            auto image_memory_barrier2 = VkImageMemoryBarrier{};
            image_memory_barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_memory_barrier2.image = image;
            image_memory_barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_memory_barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier2.subresourceRange = mip_subresource_range;
            vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier2);

        }
        subresource_range.levelCount = mip_levels;
        image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.image = image;
        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        image_memory_barrier.subresourceRange = subresource_range;
        vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        if(delete_buffer) delete[] buffer;

        device->flush_command_buffer(blit_cmd, copy_queue);

    }
    else
    {
        auto filename = path + "/" + gltf_image.uri;
        ktxTexture* ktx_texture;
        ktxResult result = KTX_SUCCESS;
        if(!uka::tools::file_exists(filename))
        {
            uka::tools::exitFatal("File not found: " + filename,-1);
        }
        result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
        assert(result == KTX_SUCCESS);

        this->device = device;
        width = ktx_texture->baseWidth;
        height = ktx_texture->baseHeight;
        mip_levels = ktx_texture->numLevels;

        auto* ktx_texture_data = ktxTexture_GetData(ktx_texture);
        auto ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

        format = VK_FORMAT_R8G8B8A8_UNORM;
        auto format_properties = VkFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(device->physical_device, format, &format_properties);

        auto copy_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
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

        auto buffer_copy_regions = std::vector<VkBufferImageCopy>();
        for(auto i=0;i<mip_levels;i++)
        {
            auto offset = ktx_size_t {};
            auto result = ktxTexture_GetImageOffset(ktx_texture, i, 0, 0, &offset);
            assert(result == KTX_SUCCESS);
            auto buffer_image_copy = VkBufferImageCopy{};
            buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            buffer_image_copy.imageSubresource.mipLevel = i;
            buffer_image_copy.imageSubresource.baseArrayLayer = 0;
            buffer_image_copy.imageSubresource.layerCount = 1;
            buffer_image_copy.imageExtent.width = std::max(1u, ktx_texture->baseWidth >> i);
            buffer_image_copy.imageExtent.height = std::max(1u, ktx_texture->baseHeight >> i);
            buffer_image_copy.imageExtent.depth = 1;
            buffer_image_copy.bufferOffset = offset;
            buffer_copy_regions.push_back(buffer_image_copy);
        }

        auto image_create_info = uka::init::image_create_info();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.extent = {width, height, 1};
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.arrayLayers = 1;
        image_create_info.mipLevels = mip_levels;
        image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
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

        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_subresource_range);
        vkCmdCopyBufferToImage(copy_cmd, stage_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_regions.size(), buffer_copy_regions.data());
        uka::tools::set_image_layout(copy_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image_subresource_range);
        device->flush_command_buffer(copy_cmd, copy_queue);
        this->image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkDestroyBuffer(this->device->logical_device, stage_buffer, nullptr);
        vkFreeMemory(this->device->logical_device, stage_memory, nullptr);
        ktxTexture_Destroy(ktx_texture);
    }

    auto sampler_create_info = VkSamplerCreateInfo();
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    sampler_create_info.maxAnisotropy = 1.0;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.maxAnisotropy = 8.0f;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod = mip_levels;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VK_CHECK_RESULT(vkCreateSampler(device->logical_device, &sampler_create_info, nullptr, &sampler));

    auto image_view_create_info = VkImageViewCreateInfo();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_view_create_info.subresourceRange.levelCount = mip_levels;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.image = image;
    VK_CHECK_RESULT(vkCreateImageView(device->logical_device, &image_view_create_info, nullptr, &image_view));

    descriptor.sampler = sampler;
    descriptor.imageView = image_view;
    descriptor.imageLayout = image_layout;

}

auto uka::gltf::Material::create_descriptor_set(VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout descriptor_set_layout,
    uint32_t descriptor_binding_flags) -> void
{
    auto descriptor_set_alloc_info = VkDescriptorSetAllocateInfo{};
    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.descriptorPool = descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = 1;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logical_device, &descriptor_set_alloc_info, &descriptor_set));
    auto image_desciptors = std::vector<VkDescriptorImageInfo>();
    auto write_descriptor_infos = std::vector<VkWriteDescriptorSet>();
    if(descriptor_binding_flags & uka::gltf::descriptor_binding_flags::image_base_color)
    {
        auto image_descriptor = VkDescriptorImageInfo{};
        image_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_descriptor.imageView = base_color_texture->image_view;
        image_descriptor.sampler = base_color_texture->sampler;
        image_desciptors.push_back(image_descriptor);
        auto write_descriptor_info = VkWriteDescriptorSet{};
        write_descriptor_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_info.dstSet = descriptor_set;
        write_descriptor_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_info.dstBinding = 0;
        write_descriptor_info.pImageInfo = &image_descriptor;
        write_descriptor_info.descriptorCount = 1;
        write_descriptor_infos.push_back(write_descriptor_info);
    }
    if(normal_texture && descriptor_binding_flags & uka::gltf::descriptor_binding_flags::image_normal_map)
    {
        auto image_descriptor = VkDescriptorImageInfo{};
        image_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_descriptor.imageView = normal_texture->image_view;
        image_descriptor.sampler = normal_texture->sampler;
        image_desciptors.push_back(image_descriptor);
        auto write_descriptor_info = VkWriteDescriptorSet{};
        write_descriptor_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_info.dstSet = descriptor_set;
        write_descriptor_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_info.dstBinding = 1;
        write_descriptor_info.pImageInfo = &image_descriptor;
        write_descriptor_info.descriptorCount = 1;
        write_descriptor_infos.push_back(write_descriptor_info);
    }
    vkUpdateDescriptorSets(device->logical_device, static_cast<uint32_t>(write_descriptor_infos.size()), write_descriptor_infos.data(), 0, nullptr);
}

auto uka::gltf::Primitive::set_dimensions(const glm::vec3& min, const glm::vec3& max) -> void
{
    dimensions.min = min;
    dimensions.max = max;
    dimensions.size = max - min;
    dimensions.center = min + (dimensions.size / 2.0f);
    dimensions.radius = glm::distance(min, max) / 2.0f;
}

uka::gltf::Mesh::Mesh(Uka_Device* device,glm::mat4 matrix)
{
    this->device = device;
    this->uniform_block.matrix = matrix;
    VK_CHECK_RESULT(device->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uniform_block), &uniform_buffer.buffer, &uniform_buffer.memory, &uniform_block));
    VK_CHECK_RESULT(vkMapMemory(device->logical_device, uniform_buffer.memory, 0, sizeof(uniform_block), 0, &uniform_buffer.mapped));
    uniform_buffer.descriptor = {uniform_buffer.buffer, 0, sizeof(uniform_block)};
}
uka::gltf::Mesh::~Mesh()
{
    vkDestroyBuffer(device->logical_device, uniform_buffer.buffer, nullptr);
    vkFreeMemory(device->logical_device, uniform_buffer.memory, nullptr);
    for(auto primitive : primitives)
    {
        delete primitive;
    }
}

auto uka::gltf::Node::local_matrix_from_node() -> glm::mat4
{
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
}

auto uka::gltf::Node::get_matrix() -> glm::mat4
{
    auto m = local_matrix_from_node();
    auto *p = parent;
    while(p)
    {
        m = p->local_matrix_from_node() * m;
        p = p->parent;
    }
    return m;
}

auto uka::gltf::Node::update() -> void
{
    if(mesh)
    {
        auto m = get_matrix();
        if(skin)
        {
            mesh->uniform_block.matrix = m;
            for(auto i=0;i<skin->joints.size();i++)
            {
                mesh->uniform_block.joint_matrix[i] = glm::inverse(m) * skin->joints[i]->get_matrix() * skin->inverse_bind_matrices[i];
            }
            mesh->uniform_block.joint_count = skin->joints.size();
            memcpy(mesh->uniform_buffer.mapped, &mesh->uniform_block, sizeof(mesh->uniform_block));
        }
        else
        {
            memcpy(mesh->uniform_buffer.mapped, &m, sizeof(m));
        }
    }
    for(auto child : children)
    {
        child->update();
    }
}

uka::gltf::Node::~Node()
{
    if(mesh)
    {
        delete mesh;
    }
    for(auto child : children)
    {
        delete child;
    }
}

static VkVertexInputBindingDescription vertex_input_binding_description;
static std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
static VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info;

auto uka::gltf::Vertex::input_binding_description(uint32_t biding) -> VkVertexInputBindingDescription
{
    return VkVertexInputBindingDescription{biding, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
}

auto uka::gltf::Vertex::input_attribute_description(uint32_t binding,
    uint32_t location,
    uka::gltf::VertexComponent component) -> VkVertexInputAttributeDescription
{
    switch (component)
    {
    case VertexComponent::POSITION:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, position)};
    case VertexComponent::NORMAL:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, normal)};
    case VertexComponent::UV:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, uv)};
    case VertexComponent::COLOR:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, color)};
    case VertexComponent::TANGENT:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, tangent)};
    case VertexComponent::JOINTS:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, joints_0)};
    case VertexComponent::WEIGHTS:
            return VkVertexInputAttributeDescription{location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t>offsetof(Vertex, weights_0)};
    default:
        return VkVertexInputAttributeDescription{};
    }
}

auto uka::gltf::Vertex::input_attribute_descriptions(uint32_t binding,
    const std::vector<VertexComponent> components) -> std::vector<VkVertexInputAttributeDescription>
{
    auto result = std::vector<VkVertexInputAttributeDescription>();
    for(auto i=0;i<components.size();i++)
    {
        result.push_back(input_attribute_description(binding, i, components[i]));
    }
    return result;
}

auto uka::gltf::Vertex::pipeline_vertex_input_state(const std::vector<VertexComponent> components) -> VkPipelineVertexInputStateCreateInfo*
{
    vertex_input_binding_description = input_binding_description(0);
    vertex_input_attribute_descriptions = input_attribute_descriptions(0, components);
    pipeline_vertex_input_state_create_info = VkPipelineVertexInputStateCreateInfo{};
    pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_descriptions.size());
    pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();
    return &pipeline_vertex_input_state_create_info;
}

auto uka::gltf::Model::get_texture(uint32_t index) -> Texture*
{
    if(index < textures.size())
    {
        return &textures[index];
    }
    return &empty_texture;
}

auto uka::gltf::Model::create_empty_texture(VkQueue queue) -> void
{
    empty_texture.device = device;
    empty_texture.width = 1;
    empty_texture.height = 1;
    empty_texture.mip_levels = 1;
    empty_texture.layer_count = 1;

    auto buffer_size = empty_texture.width * empty_texture.height * 4;
    auto buffer = new unsigned char[buffer_size];
    memset(buffer, 0, buffer_size);

    auto staging_buffer = VkBuffer{};
    auto staging_memory = VkDeviceMemory{};
    auto buffer_create_info = uka::init::buffer_create_info();
    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(device->logical_device, &buffer_create_info, nullptr, &staging_buffer));

    auto mem_alloc_info = uka::init::memory_allocate_info();
    auto mem_reqs = VkMemoryRequirements{};
    vkGetBufferMemoryRequirements(device->logical_device, staging_buffer, &mem_reqs);
    mem_alloc_info.allocationSize = mem_reqs.size;
    mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device->logical_device, &mem_alloc_info, nullptr, &staging_memory));
    VK_CHECK_RESULT(vkBindBufferMemory(device->logical_device, staging_buffer, staging_memory, 0));

    // Copy texture data into staging buffer
    uint8_t* data;
    VK_CHECK_RESULT(vkMapMemory(device->logical_device, staging_memory, 0, mem_reqs.size, 0, (void**)&data));
    memcpy(data, buffer, buffer_size);
    vkUnmapMemory(device->logical_device, staging_memory);

    auto buffer_copy_region = VkBufferImageCopy{};
    buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy_region.imageSubresource.layerCount = 1;
    buffer_copy_region.imageExtent.width = empty_texture.width;
    buffer_copy_region.imageExtent.height = empty_texture.height;
    buffer_copy_region.imageExtent.depth = 1;

    // Create optimal tiled target image
    auto image_create_info = uka::init::image_create_info();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.extent = { empty_texture.width, empty_texture.height, 1 };
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VK_CHECK_RESULT(vkCreateImage(device->logical_device, &image_create_info, nullptr, &empty_texture.image));

    vkGetImageMemoryRequirements(device->logical_device, empty_texture.image, &mem_reqs);
    mem_alloc_info.allocationSize = mem_reqs.size;
    mem_alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device->logical_device, &mem_alloc_info, nullptr, &empty_texture.device_memory));
    VK_CHECK_RESULT(vkBindImageMemory(device->logical_device, empty_texture.image, empty_texture.device_memory, 0));

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkCommandBuffer copy_cmd = device->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    uka::tools::set_image_layout(copy_cmd, empty_texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    vkCmdCopyBufferToImage(copy_cmd, staging_buffer, empty_texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);
    uka::tools::set_image_layout(copy_cmd, empty_texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
    device->flush_command_buffer(copy_cmd, queue);
    empty_texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Clean up staging resources
    vkDestroyBuffer(device->logical_device, staging_buffer, nullptr);
    vkFreeMemory(device->logical_device, staging_memory, nullptr);

    auto sample_create_info = uka::init::sampler_create_info();
    sample_create_info.magFilter = VK_FILTER_LINEAR;
    sample_create_info.minFilter = VK_FILTER_LINEAR;
    sample_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sample_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sample_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sample_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sample_create_info.compareOp = VK_COMPARE_OP_NEVER;
    sample_create_info.maxAnisotropy = 1.0f;
    VK_CHECK_RESULT(vkCreateSampler(device->logical_device, &sample_create_info, nullptr, &empty_texture.sampler));

    auto view_create_info =uka::init::image_view_create_info();
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_create_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.image = empty_texture.image;
    VK_CHECK_RESULT(vkCreateImageView(device->logical_device, &view_create_info, nullptr, &empty_texture.image_view));

    empty_texture.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    empty_texture.descriptor.imageView = empty_texture.image_view;
    empty_texture.descriptor.sampler = empty_texture.sampler;

}

uka::gltf::Model::~Model()
{
    vkDestroyBuffer(device->logical_device, vertices.buffer, nullptr);
    vkFreeMemory(device->logical_device, vertices.memory, nullptr);
    vkDestroyBuffer(device->logical_device, indices.buffer, nullptr);
    vkFreeMemory(device->logical_device, indices.memory, nullptr);
    for(auto& texture : textures)
    {
        texture.destroy();
    }
    for(auto& node : nodes)
    {
        delete node;
    }
    for(auto& skin : skins)
    {
        delete skin;
    }
    if(descriptor_set_layout_ubo != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device->logical_device, descriptor_set_layout_ubo, nullptr);
    }
    if(descriptor_set_layout_image != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device->logical_device, descriptor_set_layout_image, nullptr);
    }
    vkDestroyDescriptorPool(device->logical_device, descriptor_pool, nullptr);
    empty_texture.destroy();
}

auto uka::gltf::Model::loadNode(uka::gltf::Node* parent,
    const tinygltf::Node& node,
    uint32_t nodeIndex,
    const tinygltf::Model& model,
    std::vector<uint32_t>& indexBuffer,
    std::vector<Vertex>& vertexBuffer,
    float globalscale) -> void
{
    auto *newnode = new uka::gltf::Node();
    newnode->index = nodeIndex;
    newnode->parent = parent;
    newnode->name = node.name;
    newnode->skin_index = node.skin;
    newnode->matrix = glm::mat4(1.0f);

    if(node.translation.size()==3)
    {
        auto translation = glm::make_vec3(node.translation.data());
        newnode->translation = translation;
    }
    if(node.rotation.size()==4)
    {
        auto rotation = glm::make_quat(node.rotation.data());
        newnode->rotation = rotation;
    }
    if(node.scale.size()==3)
    {
        auto scale = glm::make_vec3(node.scale.data());
        newnode->scale = scale;
    }
    if(node.matrix.size()==16)
    {
        newnode->matrix = glm::make_mat4x4(node.matrix.data());
    }
    if(node.children.size()>0)
    {
        for(auto childIndex : node.children)
        {
            loadNode(newnode, model.nodes[childIndex], childIndex, model, indexBuffer, vertexBuffer, globalscale);
        }
    }
    if(node.mesh >-1)
    {
        const auto mesh = model.meshes[node.mesh];
        auto newmesh = new uka::gltf::Mesh(device,newnode->matrix);
        newmesh->name = mesh.name;
        for(auto j=0;j<mesh.primitives.size();j++)
        {
            auto& primitive = mesh.primitives[j];
            if(primitive.indices<0)
            {
                continue;
            }
            auto indexStart = static_cast<uint32_t>(indexBuffer.size());
            auto vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            auto indexCount = 0;
            auto vertexCount = 0;
            auto pos_min = glm::vec3(std::numeric_limits<float>::max());
            auto pos_max = glm::vec3(-std::numeric_limits<float>::max());
            bool has_skin = false;

            //Vertices
            {
                const float* buffer_pos = nullptr;
                const float* buffer_normals = nullptr;
                const float* buffer_tangents = nullptr;
                const float* buffer_texcoords = nullptr;
                const float* buffer_colors = nullptr;
                const uint16_t* buffer_joints = nullptr;
                const float* buffer_weights = nullptr;
                uint32_t num_color_component;

                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                auto pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                auto pos_view = model.bufferViews[pos_accessor.bufferView];
                buffer_pos = reinterpret_cast<const float*>(&(model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));
                pos_max = glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]);
                pos_min = glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]);

                if(primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    auto normals_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    auto normals_view = model.bufferViews[normals_accessor.bufferView];
                    buffer_normals = reinterpret_cast<const float*>(&(model.buffers[normals_view.buffer].data[normals_accessor.byteOffset + normals_view.byteOffset]));
                }
                if(primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    auto tangents_accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
                    auto tangents_view = model.bufferViews[tangents_accessor.bufferView];
                    buffer_tangents = reinterpret_cast<const float*>(&(model.buffers[tangents_view.buffer].data[tangents_accessor.byteOffset + tangents_view.byteOffset]));
                }
                if(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    auto texcoords_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    auto texcoords_view = model.bufferViews[texcoords_accessor.bufferView];
                    buffer_texcoords = reinterpret_cast<const float*>(&(model.buffers[texcoords_view.buffer].data[texcoords_accessor.byteOffset + texcoords_view.byteOffset]));
                }
                if(primitive.attributes.find("COLOR_0") != primitive.attributes.end())
                {
                    auto colors_accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
                    auto colors_view = model.bufferViews[colors_accessor.bufferView];
                    buffer_colors = reinterpret_cast<const float*>(&(model.buffers[colors_view.buffer].data[colors_accessor.byteOffset + colors_view.byteOffset]));
                    num_color_component = colors_accessor.type;
                }
                if(primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
                {
                    auto joints_accessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                    auto joints_view = model.bufferViews[joints_accessor.bufferView];
                    buffer_joints = reinterpret_cast<const uint16_t*>(&(model.buffers[joints_view.buffer].data[joints_accessor.byteOffset + joints_view.byteOffset]));
                    has_skin = true;
                }
                if(primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
                {
                    auto weights_accessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                    auto weights_view = model.bufferViews[weights_accessor.bufferView];
                    buffer_weights = reinterpret_cast<const float*>(&(model.buffers[weights_view.buffer].data[weights_accessor.byteOffset + weights_view.byteOffset]));
                    has_skin = true;
                }
                has_skin = (buffer_joints && buffer_weights);

                vertexCount = static_cast<uint32_t>(pos_accessor.count);

                for(auto v=0;v<pos_accessor.count;v++)
                {
                    auto vert = Vertex{};
                    vert.position = glm::vec4(glm::make_vec3(&buffer_pos[v * 3]), 1.0f);
                    vert.normal  =glm::normalize(glm::vec3(buffer_normals ? glm::make_vec3(&buffer_normals[v * 3]) : glm::vec3(0.0f)));
                    vert.uv = buffer_texcoords ? glm::make_vec2(&buffer_texcoords[v * 2]) : glm::vec2(0.0f);
                    if(buffer_colors)
                    {
                        switch(num_color_component)
                        {
                            case 3:
                                vert.color = glm::vec4(glm::make_vec3(&buffer_colors[v * 3]), 1.0f);
                                break;
                            case 4:
                                vert.color = glm::make_vec4(&buffer_colors[v * 4]);
                                break;
                            default:
                                vert.color = glm::vec4(1.0f);

                        }
                    }
                    else
                    {
                        vert.color = glm::vec4(1.0f);
                    }
                    vert.tangent = buffer_tangents ? glm::vec4(glm::make_vec3(&buffer_tangents[v * 3]), 1.0f) : glm::vec4(0.0f);
                    if(has_skin)
                    {
                        vert.joints_0 = glm::vec4(buffer_joints[v * 4 + 0], buffer_joints[v * 4 + 1], buffer_joints[v * 4 + 2], buffer_joints[v * 4 + 3]);
                        vert.weights_0 = glm::vec4(buffer_weights[v * 4 + 0], buffer_weights[v * 4 + 1], buffer_weights[v * 4 + 2], buffer_weights[v * 4 + 3]);
                    }
                    else
                    {
                        vert.joints_0 = glm::vec4(0.0f);
                        vert.weights_0 = glm::vec4(0.0f);
                    }
                    vertexBuffer.push_back(vert);
                }
                // Indices
                {
                    auto index_accessor = model.accessors[primitive.indices];
                    auto index_view = model.bufferViews[index_accessor.bufferView];
                    auto index_buffer = model.buffers[index_view.buffer];
                    indexCount = static_cast<uint32_t>(index_accessor.count);
                    switch(index_accessor.componentType)
                    {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            auto buf = new uint32_t[index_accessor.count];
                            memcpy(buf, &index_buffer.data[index_accessor.byteOffset+index_view.byteOffset], index_accessor.count * sizeof(uint32_t));
                            for(auto i=0;i<index_accessor.count;i++)
                            {
                                indexBuffer.push_back(buf[i] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            auto buf = new uint16_t[index_accessor.count];
                            memcpy(buf, &index_buffer.data[index_accessor.byteOffset+index_view.byteOffset], index_accessor.count * sizeof(uint16_t));
                            for(auto i=0;i<index_accessor.count;i++)
                            {
                                indexBuffer.push_back(buf[i] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            auto buf = new uint8_t[index_accessor.count];
                            memcpy(buf, &index_buffer.data[index_accessor.byteOffset+index_view.byteOffset], index_accessor.count * sizeof(uint8_t));
                            for(auto i=0;i<index_accessor.count;i++)
                            {
                                indexBuffer.push_back(buf[i] + vertexStart);
                            }
                            delete[] buf;
                            break;
                        }
                        default:
                            throw std::runtime_error("Index component type not supported");
                            return;
                    }
                }
                auto new_primitive = new Primitive(indexStart,indexCount,primitive.material > -1 ? materials[primitive.material] : materials.back());
                new_primitive->first_index = vertexStart;
                new_primitive->index_count = vertexCount;
                new_primitive->set_dimensions(pos_min, pos_max);
                newmesh->primitives.push_back(new_primitive);
            }
            newnode->mesh = newmesh;
        }
        if(parent){
            parent->children.push_back(newnode);
        }
        else
        {
            nodes.push_back(newnode);
        }
        linear_nodes.push_back(newnode);

    }

}