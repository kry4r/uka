#include "uka-tools.hpp"

auto getAssetPath()->const std::string
{
    return "./../assets/";
}
auto getShaderBasePath()->const std::string
{
    return "./../shaders/";
}

namespace uka{
    namespace tools{
        bool errorMode = false;

        auto error_string(VkResult errorCode)->std::string{
            switch(errorCode)
            {
                // Success codes
                case VK_SUCCESS:
                    return "VK_SUCCESS";
                case VK_NOT_READY:
                    return "VK_NOT_READY";
                case VK_TIMEOUT:
                    return "VK_TIMEOUT";
                case VK_EVENT_SET:
                    return "VK_EVENT_SET";
                case VK_EVENT_RESET:
                    return "VK_EVENT_RESET";
                case VK_INCOMPLETE:
                    return "VK_INCOMPLETE";
                case VK_SUBOPTIMAL_KHR:
                    return "VK_SUBOPTIMAL_KHR";
                // Error codes
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return "VK_ERROR_OUT_OF_HOST_MEMORY";
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
                case VK_ERROR_INITIALIZATION_FAILED:
                    return "VK_ERROR_INITIALIZATION_FAILED";
                case VK_ERROR_DEVICE_LOST:
                    return "VK_ERROR_DEVICE_LOST";
                case VK_ERROR_MEMORY_MAP_FAILED:
                    return "VK_ERROR_MEMORY_MAP_FAILED";
                case VK_ERROR_LAYER_NOT_PRESENT:
                    return "VK_ERROR_LAYER_NOT_PRESENT";
                case VK_ERROR_EXTENSION_NOT_PRESENT:
                    return "VK_ERROR_EXTENSION_NOT_PRESENT";
                case VK_ERROR_FEATURE_NOT_PRESENT:
                    return "VK_ERROR_FEATURE_NOT_PRESENT";
                case VK_ERROR_INCOMPATIBLE_DRIVER:
                    return "VK_ERROR_INCOMPATIBLE_DRIVER";
                case VK_ERROR_TOO_MANY_OBJECTS:
                    return "VK_ERROR_TOO_MANY_OBJECTS";
                case VK_ERROR_FORMAT_NOT_SUPPORTED:
                    return "VK_ERROR_FORMAT_NOT_SUPPORTED";
                case VK_ERROR_FRAGMENTED_POOL:
                    return "VK_ERROR_FRAGMENTED_POOL";
                case VK_ERROR_UNKNOWN:
                    return "VK_ERROR_UNKNOWN";
                case VK_ERROR_OUT_OF_POOL_MEMORY:
                    return "VK_ERROR_OUT_OF_POOL_MEMORY";
                case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                    return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
                case VK_ERROR_FRAGMENTATION:
                    return "VK_ERROR_FRAGMENTATION";
                case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
                    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
                case VK_ERROR_SURFACE_LOST_KHR:
                    return "VK_ERROR_SURFACE_LOST_KHR";
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
                case VK_ERROR_OUT_OF_DATE_KHR:
                    return "VK_ERROR_OUT_OF_DATE_KHR";
                case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
                default:
                    return "UNKNOWN_ERROR";
            }
        }
        auto physical_device_type_string(VkPhysicalDeviceType type)->std::string
        {
            switch(type)
            {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    return "VK_PHYSICAL_DEVICE_TYPE_CPU";
                default:
                    return "UNKNOWN_DEVICE_TYPE";
            }
        }
        auto get_supported_depth_format(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)->VkFormat
        {
            auto format_list = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM
            };

            for(auto& format : format_list)
            {
                auto props = VkFormatProperties{};
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
                if(props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    *depthFormat = format;
                    return format;
                }
            }
        }
        auto get_supported_depth_stencial_format(VkPhysicalDevice physicalDevice,VkFormat* depthStencilFormat)->VkFormat
        {
            auto format_list = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM
            };

            for(auto& format : format_list)
            {
                auto props = VkFormatProperties{};
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
                if(props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    *depthStencilFormat = format;
                    return format;
                }
            }
        }
        auto format_is_filterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)->bool
        {
            auto props = VkFormatProperties{};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if(tiling == VK_IMAGE_TILING_OPTIMAL)
            {
                return (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
            }
            else if(tiling == VK_IMAGE_TILING_LINEAR)
            {
                return (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
            }
            return false;
        }
        auto format_has_stencil(VkFormat format)->bool
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_S8_UINT;
        }

        auto set_image_layout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageSubresourceRange subresourceRange,VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask)->void
                            {
                                auto image_memory_barrier = init::image_memory_barrier();
                                image_memory_barrier.oldLayout = old_image_layout;
                                image_memory_barrier.newLayout = new_image_layout;
                                image_memory_barrier.image = image;
                                image_memory_barrier.subresourceRange = subresourceRange;

                                switch(old_image_layout)
                                {
                                    case VK_IMAGE_LAYOUT_PREINITIALIZED:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                                        image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                        break;
                                    default:
                                        image_memory_barrier.srcAccessMask = 0;
                                        break;
                                }

                                switch(new_image_layout)
                                {
                                    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                                        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                                        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                                        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                                        image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                                        break;
                                    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                                        if(image_memory_barrier.srcAccessMask == 0)
                                        {
                                            image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                                        }
                                        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                        break;
                                    default:
                                        image_memory_barrier.dstAccessMask = 0;
                                        break;
                                }

                                vkCmdPipelineBarrier(
                                    cmdbuffer,
                                    srcStageMask,
                                    dstStageMask,
                                    0,
                                    0, nullptr,
                                    0, nullptr,
                                    1, &image_memory_barrier
                                );
                            }
        auto set_image_layout(VkCommandBuffer cmdbuffer, VkImage image,VkImageAspectFlags aspec_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout,VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask)->void{
                                auto subresourceRange = VkImageSubresourceRange{};
                                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                subresourceRange.baseMipLevel = 0;
                                subresourceRange.levelCount = 1;
                                subresourceRange.layerCount = 1;
                                set_image_layout(cmdbuffer, image,old_image_layout, new_image_layout, subresourceRange, srcStageMask, dstStageMask);
                            }
        auto insert_image_memort_barrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)->void
        {
            auto image_memory_barrier = init::image_memory_barrier();
            image_memory_barrier.srcAccessMask = srcAccessMask;
            image_memory_barrier.dstAccessMask = dstAccessMask;
            image_memory_barrier.oldLayout = oldImageLayout;
            image_memory_barrier.newLayout = newImageLayout;
            image_memory_barrier.image = image;
            image_memory_barrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        auto exitFatal(std::string message, uint32_t errorCode)->void
        {
            std::cerr << message << "\n";
        }
        auto exitFatal(std::string message, VkResult errorCode)->void
        {
            exitFatal(message, (int32_t)errorCode);
        }

        auto load_shader(std::string filename,VkDevice device)->VkShaderModule
        {
            std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

			if (is.is_open())
			{
				size_t size = is.tellg();
				is.seekg(0, std::ios::beg);
				char* shaderCode = new char[size];
				is.read(shaderCode, size);
				is.close();

				assert(size > 0);

				VkShaderModule shaderModule;
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = size;
				moduleCreateInfo.pCode = (uint32_t*)shaderCode;

				VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

				delete[] shaderCode;

				return shaderModule;
			}
			else
			{
				std::cerr << "Error: Could not open shader file \"" << filename << "\"" << "\n";
				return VK_NULL_HANDLE;
			}
        }

        auto file_exists(const std::string &filename) -> bool
        {
            std::ifstream f(filename.c_str());
			return !f.fail();
        }

        auto aligned_size(uint32_t value, uint32_t alignment) -> uint32_t
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }
        auto aligned_vk_size(uint32_t value, uint32_t alignment) -> VkDeviceSize
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }
    }
}
