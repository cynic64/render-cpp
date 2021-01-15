#ifndef LL_QUEUE_H
#define LL_QUEUE_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace ll::queue {
	struct QueueFamilies {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		std::vector<uint32_t> unique;

		QueueFamilies() = default;

		// Surface can be empty if you don't care about present support
		QueueFamilies(VkPhysicalDevice phys_dev, std::optional<VkSurfaceKHR> surface = std::nullopt);
	};

	// Any that don't exist will be VK_NULL_HANDLE
	struct Queues {
		VkQueue graphics = VK_NULL_HANDLE;
		VkQueue present = VK_NULL_HANDLE;

		Queues(VkDevice device, QueueFamilies queue_fams);
		Queues() = default;
	};
}

#endif // LL_QUEUE_H
