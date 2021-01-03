#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vk_swapchain {
	struct SwapchainSettings {
		VkFormat format_pref;
		VkColorSpaceKHR color_space_pref;
		VkPresentModeKHR present_mode_pref;
		VkImageUsageFlags image_usage;
		uint32_t image_ct_pref;
	};

	const SwapchainSettings DEFAULT_SWAPCHAIN {
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		3
	};

	struct Swapchain {
	public:
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;

		VkFormat format;
		VkColorSpaceKHR color_space;
		VkPresentModeKHR present_mode;
		uint32_t width;
		uint32_t height;

		// Queue_fam_ct should be less than 2 if
		// VK_SHARING_MODE_EXCLUSIVE is desired. Queue_fams can be
		// invalidated after create() is finished. Old_swapchain should
		// be VK_NULL_HANDLE if none exists.
		//
		// The preferences in settings may not be possible, look at our
		// fields to see what format and so on was really chosen.
		Swapchain(VkPhysicalDevice phys_dev, VkDevice device,
			  VkSurfaceKHR surface, VkSwapchainKHR old_swapchain,
			  uint32_t queue_fam_ct, uint32_t* queue_fams,
			  uint32_t window_width, uint32_t window_height,
			  SwapchainSettings const& settings = DEFAULT_SWAPCHAIN);

		void destroy();

	private:
		VkDevice device;
	};

}

#endif // VK_SWAPCHAIN_H
