#include "uka-swapchain.hpp"


#if defined(VK_USE_PLATFORM_WIN32_KHR)
void uka::SwapChain::initSurface(void* platformHandle, void* platformWindow)
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
void uka::SwapChain::initSurface(uint32_t width, uint32_t height)
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
void uka::SwapChain::initSurface(screen_context_t screen_context, screen_window_t screen_window)
#endif
{
    VkResult err = VK_SUCCESS;

	// Create the os-specific surface
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
	surfaceCreateInfo.hwnd = (HWND)platformWindow;
	err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(_DIRECT2DISPLAY)
	createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
	VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
	PFN_vkCreateHeadlessSurfaceEXT fpCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
	if (!fpCreateHeadlessSurfaceEXT){
		uka::tools::exitFatal("Could not fetch function pointer for the headless extension!", -1);
	}
	err = fpCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	VkScreenSurfaceCreateInfoQNX surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.context = screen_context;
	surfaceCreateInfo.window = screen_window;
	err = vkCreateScreenSurfaceQNX(instance, &surfaceCreateInfo, NULL, &surface);
#endif

	if (err != VK_SUCCESS) {
		uka::tools::exitFatal("Could not create surface!", err);
	}

	// Get available queue family properties
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueCount, nullptr);
	assert(queueCount >= 1);

	std::vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
	{
		uka::tools::exitFatal("Could not find a graphics and/or presenting queue!", -1);
	}

	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
	{
		uka::tools::exitFatal("Separate graphics and presenting queues are not supported yet!", -1);
	}

	queue_node_index = graphicsQueueNodeIndex;

	// Get list of supported surface formats
	uint32_t formatCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr));
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, surfaceFormats.data()));

	// We want to get a format that best suits our needs, so we try to get one from a set of preferred formats
	// Initialize the format to the first one returned by the implementation in case we can't find one of the preffered formats
	VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];
	std::vector<VkFormat> preferredImageFormats = {
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_A8B8G8R8_UNORM_PACK32
	};

	for (auto& availableFormat : surfaceFormats) {
		if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
			selectedFormat = availableFormat;
			break;
		}
	}

	color_format = selectedFormat.format;
	color_space = selectedFormat.colorSpace;
}
auto uka::SwapChain::set_context(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device) -> void
{
    this->instance = instance;
    this->device = device;
    this->physical_device = physical_device;
}


auto uka::SwapChain::create(uint32_t width, uint32_t height, bool vsync,bool fullscreen) ->void
{
    assert(physical_device);
    assert(device);
    assert(instance);

    auto old_swapchain = swap_chain;

    auto surface_caps = VkSurfaceCapabilitiesKHR{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_caps));

    auto present_mode_count = uint32_t{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,nullptr));
    assert(present_mode_count > 0);

    auto present_modes = std::vector<VkPresentModeKHR>(present_mode_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data()));

    auto swapchain_extent = VkExtent2D{};

    if (surface_caps.currentExtent.width == -1)
    {
        swapchain_extent.width = width;
        swapchain_extent.height = height;
    }
    else
    {
        swapchain_extent = surface_caps.currentExtent;
    }

    auto swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if(!vsync)
    {
        for(auto& present_mode : present_modes)
        {
            if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    auto desired_image_count = surface_caps.minImageCount + 1;

    if((surface_caps.maxImageCount > 0) && (desired_image_count > surface_caps.maxImageCount))
    {
        desired_image_count = surface_caps.maxImageCount;
    }

    auto pre_transform = VkSurfaceTransformFlagBitsKHR{};
    if(surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        pre_transform = surface_caps.currentTransform;
    }

    auto composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    auto composite_alpha_flags = std::vector<VkCompositeAlphaFlagBitsKHR>{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    };

    for(auto& composite_alpha_flag : composite_alpha_flags)
    {
        if(surface_caps.supportedCompositeAlpha & composite_alpha_flag)
        {
            composite_alpha = composite_alpha_flag;
            break;
        }
    }

    auto swapchain_ci = VkSwapchainCreateInfoKHR{};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = surface;
    swapchain_ci.minImageCount = desired_image_count;
    swapchain_ci.imageFormat = color_format;
    swapchain_ci.imageColorSpace = color_space;
    swapchain_ci.imageExtent = swapchain_extent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = pre_transform;
    swapchain_ci.compositeAlpha = composite_alpha;
    swapchain_ci.presentMode = swapchain_present_mode;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.oldSwapchain = old_swapchain;

    if(surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if(surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchain_ci, nullptr, &swap_chain));

    if(old_swapchain != VK_NULL_HANDLE)
    {
        for(auto& image_view : image_views)
        {
            vkDestroyImageView(device, image_view, nullptr);
        }
        vkDestroySwapchainKHR(device, old_swapchain, nullptr);
    }
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr));

    images.resize(image_count);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, images.data()));

    buffers.resize(image_count);
    for(auto i = 0; i < image_count; i++)
    {
        auto color_image_view = VkImageView{};
        auto color_image_view_ci = VkImageViewCreateInfo{};
        color_image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_image_view_ci.format = color_format;
        color_image_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
        color_image_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
        color_image_view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
        color_image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
        color_image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_image_view_ci.subresourceRange.baseMipLevel = 0;
        color_image_view_ci.subresourceRange.levelCount = 1;
        color_image_view_ci.subresourceRange.baseArrayLayer = 0;
        color_image_view_ci.subresourceRange.layerCount = 1;
        color_image_view_ci.image = images[i];
        VK_CHECK_RESULT(vkCreateImageView(device, &color_image_view_ci, nullptr, &color_image_view));
        buffers[i].image = images[i];
        buffers[i].view = color_image_view;
    }

}

auto uka::SwapChain::acquire_next_image(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex) -> VkResult
{
    return vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
}

auto uka::SwapChain::queue_present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) -> VkResult
{
    auto present_info = VkPresentInfoKHR{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swap_chain;
    present_info.pImageIndices = &imageIndex;
    if(waitSemaphore != VK_NULL_HANDLE)
    {
        present_info.pWaitSemaphores = &waitSemaphore;
        present_info.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &present_info);
}

auto uka::SwapChain::clean_up() -> void
{
    if(swap_chain != VK_NULL_HANDLE)
    {
        for(auto& image_view : image_views)
        {
            vkDestroyImageView(device, image_view, nullptr);
        }
        vkDestroySwapchainKHR(device, swap_chain, nullptr);
    }
    if(surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    surface = VK_NULL_HANDLE;
    swap_chain = VK_NULL_HANDLE;
}


