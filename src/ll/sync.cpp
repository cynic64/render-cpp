#include "sync.hpp"

namespace ll::sync {
	VkSemaphore semaphore(VkDevice device) {
		VkSemaphore sem;
		vkCreateSemaphore(device, &DEFAULT_SEM, nullptr, &sem);
		return sem;
	}

	VkFence fence(VkDevice device, VkFenceCreateFlags flags) {
		auto info = DEFAULT_FENCE;
		info.flags |= flags;
		
		VkFence fence;
		vkCreateFence(device, &info, nullptr, &fence);
		return fence;
	}
}
