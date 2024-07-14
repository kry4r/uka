#pragma once

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <vulkan/vulkan.h>
#include "uka-tools.hpp"

#define VK_USE_PLATFORM_HEADLESS_EXT

namespace uka
{
    typedef struct _SwapChainBuffers {
    VkImage image;
    VkImageView view;
} swapchain_buffer;

    struct SwapChain
    {
    private:
        VkInstance instance;
        VkDevice device;
        VkPhysicalDevice physical_device;
        VkSurfaceKHR surface;
    public:
        VkFormat color_format;
        VkColorSpaceKHR color_space;
        VkSwapchainKHR swap_chain;
        uint32_t image_count;
        std::vector<swapchain_buffer> buffers;
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        uint32_t queue_node_index = UINT32_MAX;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	    void initSurface(void* platformHandle, void* platformWindow);
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
	    void initSurface(uint32_t width, uint32_t height);
#if defined(_DIRECT2DISPLAY)
	    void createDirect2DisplaySurface(uint32_t width, uint32_t height);
#endif
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	    void initSurface(screen_context_t screen_context, screen_window_t screen_window);
#endif
        auto set_context(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device) -> void;
        auto create(uint32_t width, uint32_t height, bool vsync = false,bool fullscreen = false) -> void;
        auto acquire_next_image(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex) -> VkResult;
        auto queue_present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE) -> VkResult;
        auto clean_up() -> void;
    };
}

