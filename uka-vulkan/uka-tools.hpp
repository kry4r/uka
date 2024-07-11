#pragma once

#include "vulkan/vulkan.h"
#include "uka-vk-init.hpp"

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <algorithm>

#define VK_FLAGS_NONE 0

#define DEFAULT_FENCE_TIMEOUT 100000000000

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << uka::tools::error_string(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
auto getAssetPath()->const std::string;
auto getShaderBasePath()->const std::string;

namespace uka
{
    namespace tools
    {
        extern bool errorMode;
        auto error_string(VkResult errorCode)->std::string;
        auto physical_device_type_string(VkPhysicalDeviceType type)->std::string;
        auto get_supported_depth_format(VkPhysicalDevice physicalDevice ,VkFormat *depthFormat)->VkFormat;
        auto get_supported_depth_stencial_format(VkPhysicalDevice physicalDevice,VkFormat* depthStencilFormat)->VkFormat;
        auto format_is_filterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)->bool;
        auto format_has_stencil(VkFormat format)->bool;

        auto set_image_layout(VkCommandBuffer cmdbuffer, VkImage image,  VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageSubresourceRange subresourceRange,VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)->void;
        auto set_image_layout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,VkImageLayout old_image_layout, VkImageLayout new_image_layout,VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)->void;
        auto insert_image_memort_barrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)->void;

        auto exitFatal(std::string message, uint32_t errorCode)->void;
        auto exitFatal(std::string message, VkResult errorCode)->void;

        auto load_shader(std::string filename,VkDevice device)->VkShaderModule;

        auto file_exists(const std::string &filename) -> bool;

        auto aligned_size(uint32_t value, uint32_t alignment) -> uint32_t;
        auto aligned_vk_size(uint32_t value, uint32_t alignment) -> VkDeviceSize;
    }//namespace tools

} // namespace uka
