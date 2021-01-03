#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vk_image {
	struct ImageViewSettings {
		VkImageSubresourceRange subresource_range;
	};

	const ImageViewSettings DEFAULT_IMAGE_VIEW {
		{ // subresource_range
			VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
			0, // baseMipLevel
			1, // levelCount
			0, // baseArrayLayer
			1  // layerCount
		}
	};

	VkImageView to_view(VkDevice device, VkImage image, VkFormat format,
			    ImageViewSettings const& settings = DEFAULT_IMAGE_VIEW);
}

#endif // VK_IMAGE_H
