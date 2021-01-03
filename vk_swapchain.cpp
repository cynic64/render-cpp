#include "vk_swapchain.hpp"

#include "vk_image.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace vk_swapchain {
	Swapchain::Swapchain(VkPhysicalDevice phys_dev, VkDevice device,
			     VkSurfaceKHR surface, VkSwapchainKHR old_swapchain,
			     uint32_t queue_fam_ct, uint32_t* queue_fams,
			     uint32_t window_width, uint32_t window_height,
			     SwapchainSettings const& settings)
		: device(device) {

		VkSurfaceCapabilitiesKHR surface_caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &surface_caps);
		
		uint32_t format_ct;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(format_ct);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, formats.data());
		
		uint32_t present_mode_ct;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, nullptr);
		std::vector<VkPresentModeKHR> present_modes(present_mode_ct);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, present_modes.data());
		
		if (format_ct == 0 || present_mode_ct == 0)
			throw std::runtime_error("Surface formats or present modes unsupported by physical device!");

		VkFormat chosen_format;
		VkColorSpaceKHR chosen_color_space;
		VkPresentModeKHR chosen_present_mode;
		if (std::any_of(formats.begin(), formats.end(),
				[&](auto f){return f.format == settings.format_pref;}))
			chosen_format = settings.format_pref;
		else chosen_format = formats[0].format;

		if (std::any_of(formats.begin(), formats.end(),
				[&](auto f){return f.colorSpace == settings.color_space_pref;}))
			chosen_color_space = settings.color_space_pref;
		else chosen_color_space = formats[0].colorSpace;

		if (std::find(present_modes.begin(), present_modes.end(), settings.present_mode_pref)
		    != present_modes.end())
			chosen_present_mode = settings.present_mode_pref;
		else chosen_present_mode = present_modes[0];

		uint32_t chosen_image_ct = std::clamp(settings.image_ct_pref,
						      surface_caps.minImageCount, surface_caps.maxImageCount);

		// Create swapchain
		VkSwapchainCreateInfoKHR swapchain_info{};
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.surface = surface;
		swapchain_info.minImageCount = chosen_image_ct;
		swapchain_info.imageFormat = chosen_format;
		swapchain_info.imageColorSpace = chosen_color_space;
		swapchain_info.imageExtent.width = std::clamp(window_width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width);
		swapchain_info.imageExtent.height = std::clamp(window_height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = settings.image_usage;
		if (queue_fam_ct >= 2) {
			swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_info.queueFamilyIndexCount = queue_fam_ct;
			swapchain_info.pQueueFamilyIndices = queue_fams;
		} else swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.preTransform = surface_caps.currentTransform;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = chosen_present_mode;
		swapchain_info.clipped = VK_TRUE;
		swapchain_info.oldSwapchain = old_swapchain;

		if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS)
			throw std::runtime_error("Could not create swapchain!");

		// Create images
		uint32_t real_image_ct; // Not necessarily what we chose
		vkGetSwapchainImagesKHR(device, swapchain, &real_image_ct, nullptr);
		images.resize(real_image_ct);
		vkGetSwapchainImagesKHR(device, swapchain, &real_image_ct, images.data());

		// Create image views
		for (auto i : images) image_views.push_back(vk_image::to_view(device, i, chosen_format));

		format = chosen_format;
		color_space = chosen_color_space;
		present_mode = chosen_present_mode;
		width = swapchain_info.imageExtent.width;
		height = swapchain_info.imageExtent.height;
	}

	void Swapchain::destroy() {
		for (auto i : image_views) vkDestroyImageView(device, i, nullptr);
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
}
