#ifndef VK_QUEUE_H
#define VK_QUEUE_H

#include <vulkan/vulkan.h>
#include <optional>
#include <unordered_set>

namespace vk_queue {
	struct QueueFamilies {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		std::unordered_set<uint32_t> unique;

		// Surface can be nullptr if you don't care about present support
		QueueFamilies(VkPhysicalDevice phys_dev, VkSurfaceKHR surface);
	};
}

#endif // VK_QUEUE_H
