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

		QueueFamilies() {};

		// Surface can be empty if you don't care about present support
		QueueFamilies(VkPhysicalDevice phys_dev, std::optional<VkSurfaceKHR> surface = std::nullopt);
	};

	// Any that don't exist will be VK_NULL_HANDLE
	struct Queues {
		VkQueue graphics;
		VkQueue present;

		Queues(VkDevice device, QueueFamilies queue_fams);
		Queues() : graphics(VK_NULL_HANDLE), present(VK_NULL_HANDLE) {}
	};
}

#endif // LL_QUEUE_H
