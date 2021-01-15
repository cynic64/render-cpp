#include "sync.hpp"

namespace ll::sync {
	auto semaphore(VkDevice device) -> VkSemaphore {
		VkSemaphore sem{};
		vkCreateSemaphore(device, &DEFAULT_SEM, nullptr, &sem);
		return sem;
	}

	auto fence(VkDevice device, VkFenceCreateFlags flags) -> VkFence {
		auto info = DEFAULT_FENCE;
		info.flags |= flags;
		
		VkFence fence{};
		vkCreateFence(device, &info, nullptr, &fence);
		return fence;
	}
}
