#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <iterator>
#include <algorithm>
#include "uka-device.hpp"
#include "uka-tools.hpp"


namespace uka
{
    struct FramebufferAttachement
    {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
        VkFormat format;
        VkImageSubresourceRange subresourceRange;
        VkAttachmentDescription description;

        auto has_depath() const -> bool
        {
            return format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT
                || format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_S8_UINT;
        }

        auto has_stencil() const -> bool
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_S8_UINT;
        }

        auto is_depth_stencil() const -> bool
        {
            return has_depath() || has_stencil();
        }
    };


    struct AttachementCreateInfo
    {
        uint32_t width,height;
        uint32_t layout_count;
        VkFormat format;
        VkImageUsageFlags usage;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    };

    struct Uka_Framebuffer
    {
    private:
        Uka_Device *device;
    public:
        uint32_t width,height;
        VkFramebuffer framebuffer;
        std::vector<FramebufferAttachement> attachements;
        VKRenderPass render_pass;
        VKSampler sampler;

        Uka_Framebuffer(Uka_Device *device)
        {
            assert(device);
            this->device = device;
        }

        ~Uka_Framebuffer()
        {
            vkDestroyFramebuffer(device->device,framebuffer,nullptr);
            for(auto &attachement : attachements)
            {
                vkDestroyImageView(device->device,attachement.view,nullptr);
                vkDestroyImage(device->device,attachement.image,nullptr);
                vkFreeMemory(device->device,attachement.memory,nullptr);
            }
        }

        auto add_attachement(uka::AttachementCreateInfo create_info)
        {
            FramebufferAttachement attachement;
            attachement.format = create_info.format;

            VkImageAspectFlags aspect_mask = VK_FLAGS_NONE;

            if(create_info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            {
                aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(create_info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                if(attachement.has_depath())
                {
                    aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
                }
                if(attachement.has_stencil())
                {
                    aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
                }
                if(attachement.is_depth_stencil())
                {
                    aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            assert(aspect_mask >0);
            auto image_info = uka::init::image_create_info();
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.format = create_info.format;
            image_info.extent.width = create_info.width;
            image_info.extent.height = create_info.height;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.samples = create_info.samples;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.usage = create_info.usage;
            VK_CHECK_RESULT(vkCreateImage(device->logical_device,&image_info,nullptr,&attachement.image));

            auto mem_reqs = VkMemoryRequirements();
            auto alloc_info = uka::init::memory_allocate_info();
            vkGetImageMemoryRequirements(device->logical_device,attachement.image,&mem_reqs);
            alloc_info.allocationSize = mem_reqs.size;
            alloc_info.memoryTypeIndex = device->get_memory_type(mem_reqs.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device->logical_device,&alloc_info,nullptr,&attachement.memory));
            VK_CHECK_RESULT(vkBindImageMemory(device->logical_device,attachement.image,attachement.memory,0));

            attachement.subresourceRange = {};
            attachement.subresourceRange.aspectMask = aspect_mask;
            attachement.subresourceRange.levelCount = 1;
            attachement.subresourceRange.layerCount = create_info.layout_count;

            auto view_info = init::image_view_create_info();
            view_info.viewType = (create_info.layout_count == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            view_info.format = create_info.format;
            view_info.subresourceRange = attachement.subresourceRange;
            view_info.subresourceRange.aspectMask = (attachement.has_depath()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspect_mask;
            view_info.image = attachement.image;
            VK_CHECK_RESULT(vkCreateImage(device->logical_device,&view_info,nullptr,&attachement.view));
        }

    };

}