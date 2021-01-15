#include "swapchain.hpp"

#include "image.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace ll::swapchain {
	auto create(VkPhysicalDevice phys_dev, VkDevice device,
		    VkSurfaceKHR surface, VkSwapchainKHR old_swapchain,
		    uint32_t queue_fam_ct, const uint32_t* queue_fams,
		    uint32_t window_width, uint32_t window_height,
		    SwapchainSettings const& settings) -> Swapchain {
		Swapchain sc{};

		VkSurfaceCapabilitiesKHR surface_caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &surface_caps);
		
		uint32_t format_ct = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(format_ct);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, formats.data());
		
		uint32_t present_mode_ct = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, nullptr);
		std::vector<VkPresentModeKHR> present_modes(present_mode_ct);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, present_modes.data());
		
		if (format_ct == 0 || present_mode_ct == 0) {
			throw std::runtime_error("Surface formats or present modes unsupported by physical device!");
		}

		sc.format = formats[0].format;
		sc.color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		sc.present_mode = present_modes[0];
		if (std::any_of(formats.begin(), formats.end(),
				[&](auto f){return f.format == settings.format_pref;})) {
			sc.format = settings.format_pref;
		}

		if (std::any_of(formats.begin(), formats.end(),
				[&](auto f){return f.colorSpace == settings.color_space_pref;})) {
			sc.color_space = settings.color_space_pref;
		}

		for (auto p : settings.present_mode_pref) {
			if (std::find(present_modes.begin(), present_modes.end(), p) != present_modes.end()) {
				sc.present_mode = p;
				break;
			}
		}

		uint32_t image_ct = std::clamp(settings.image_ct_pref,
					       surface_caps.minImageCount, surface_caps.maxImageCount);

		// Create swapchain
		VkSwapchainCreateInfoKHR swapchain_info{};
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.surface = surface;
		swapchain_info.minImageCount = image_ct;
		swapchain_info.imageFormat = sc.format;
		swapchain_info.imageColorSpace = sc.color_space;
		swapchain_info.imageExtent.width = std::clamp(window_width,
							      surface_caps.minImageExtent.width,
							      surface_caps.maxImageExtent.width);
		swapchain_info.imageExtent.height = std::clamp(window_height,
							       surface_caps.minImageExtent.height,
							       surface_caps.maxImageExtent.height);
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = settings.image_usage;
		if (queue_fam_ct >= 2) {
			swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_info.queueFamilyIndexCount = queue_fam_ct;
			swapchain_info.pQueueFamilyIndices = queue_fams;
		} else swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.preTransform = surface_caps.currentTransform;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = sc.present_mode;
		swapchain_info.clipped = VK_TRUE;
		swapchain_info.oldSwapchain = old_swapchain;

		if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &sc.handle) != VK_SUCCESS) {
			throw std::runtime_error("Could not create swapchain!");
		}

		// Create images
		uint32_t real_image_ct = 0; // Not necessarily what we chose
		vkGetSwapchainImagesKHR(device, sc.handle, &real_image_ct, nullptr);
		if (real_image_ct == 0) throw std::runtime_error("Image count is zero!");

		sc.images.resize(real_image_ct);
		vkGetSwapchainImagesKHR(device, sc.handle, &real_image_ct, sc.images.data());

		// Create image views
		for (auto i : sc.images) sc.image_views.push_back(ll::image::to_view(device, i, sc.format));

		sc.format = sc.format;
		sc.color_space = sc.color_space;
		sc.present_mode = sc.present_mode;
		sc.width = swapchain_info.imageExtent.width;
		sc.height = swapchain_info.imageExtent.height;

		return sc;
	}

	void destroy(VkDevice device, const Swapchain& swapchain) {
		for (auto i : swapchain.image_views) vkDestroyImageView(device, i, nullptr);
		vkDestroySwapchainKHR(device, swapchain.handle, nullptr);
	}
}
