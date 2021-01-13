#ifndef Ll_IMAGE_H
#define Ll_IMAGE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace ll::image {
	struct ImageViewSettings {
		VkImageSubresourceRange subresource_range;
	};

	const ImageViewSettings IMAGE_VIEW_DEFAULTS {
		{
			VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
			0, // baseMipLevel
			1, // levelCount
			0, // baseArrayLayer
			1  // layerCount
		}
	};

	VkImageView to_view(VkDevice device, VkImage image, VkFormat format,
			    ImageViewSettings const& settings = IMAGE_VIEW_DEFAULTS);
}

#endif // Ll_IMAGE_H
