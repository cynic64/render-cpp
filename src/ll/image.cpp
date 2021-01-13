#include "image.hpp"

#include <stdexcept>

namespace ll::image {
	std::vector<VkImage> from_swapchain(VkDevice device, VkSwapchainKHR swapchain) {
		uint32_t image_ct;
		vkGetSwapchainImagesKHR(device, swapchain, &image_ct, nullptr);
		std::vector<VkImage> images(image_ct);
		vkGetSwapchainImagesKHR(device, swapchain, &image_ct, images.data());

		return images;
	}

	VkImageView to_view(VkDevice device, VkImage image, VkFormat format,
			    ImageViewSettings const& settings) {
		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		info.image = image;
		info.format = format;
		info.subresourceRange = settings.subresource_range;

		VkImageView view;
		if (vkCreateImageView(device, &info, nullptr, &view) != VK_SUCCESS)
			throw std::runtime_error("Could not create image view!");

		return view;
	}
}
