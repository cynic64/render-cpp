#ifndef LL_SYNC_H
#define LL_SYNC_H

#include <vulkan/vulkan.h>

namespace ll::sync {
	const VkSemaphoreCreateInfo DEFAULT_SEM {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr, // pNext
		0 // flags
	};

	const VkFenceCreateInfo DEFAULT_FENCE {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr, // pNext
		0 // flags
	};

	auto semaphore(VkDevice device) -> VkSemaphore;

	auto fence(VkDevice device, VkFenceCreateFlags flags = 0) -> VkFence;
}

#endif // LL_SYNC_H
