#ifndef VK_QUEUE_H
#define VK_QUEUE_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace vk_queue {
	struct QueueFamilies {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		std::vector<uint32_t> unique;

		QueueFamilies() {};
		// Surface can be empty if you don't care about present support
		QueueFamilies(VkPhysicalDevice phys_dev, std::optional<VkSurfaceKHR> surface = std::nullopt);
	};
}

#endif // VK_QUEUE_H
