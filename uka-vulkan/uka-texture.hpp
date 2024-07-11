#pragma once

#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"
#include "ktx.h"
#include "ktxvulkan.h"

#include "uka-buffer.hpp"
#include "uka-device.hpp"
#include "uka-tools.hpp"

namespace uka
{
    struct Uka_Texture
    {
        Uka_Device* device;
        VkImage image;
        VkImageLayout image_layout;
        VkDeviceMemory device_memory;
        VkImageView image_view;
        uint32_t width, height;
        uint32_t mip_levels;
        uint32_t layer_count;
        VkDescriptorImageInfo descriptor;
        VkSampler sampler;

        auto update_descriptor() -> void;
        auto destroy() -> void;
        auto load_ktx(std::string file_path,ktxTexture** ktx_texture) ->ktxResult;
    };

    struct Uka_Texture2D : Uka_Texture
    {
        auto load(std::string file_path,
                    VkFormat format, Uka_Device* device,
                    VkQueue copy_queue,
                    VkImageUsageFlags img_usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                    VkImageLayout img_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    bool force_linear = false) -> void;

        auto form_buffer(void* buffer,
                        VkDeviceSize buffer_size,
                        VkFormat format,
                        uint32_t width,
                        uint32_t height,
                        Uka_Device* device,
                        VkQueue copy_queue,
                        VkImageUsageFlags img_usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                        VkImageLayout img_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VkFilter filter = VK_FILTER_LINEAR) -> void;
    };

    struct Uka_Texture2D_Array : Uka_Texture
    {
        auto load(std::string file_path,
                    VkFormat format, Uka_Device* device,
                    VkQueue copy_queue,
                    VkImageUsageFlags img_usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                    VkImageLayout img_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) -> void;
    };

    struct Uka_Texture_Cube : Uka_Texture
    {
        auto load(std::string file_path,
                    VkFormat format, Uka_Device* device,
                    VkQueue copy_queue,
                    VkImageUsageFlags img_usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                    VkImageLayout img_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) -> void;
    };
} // namespace uka
