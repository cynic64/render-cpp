#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <algorithm>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk_device {
	const float DEFAULT_QUEUE_PRIORITY = 1.0f;

	// Req_features is currently unimplemented
	void create(VkPhysicalDevice phys_dev,
		    std::vector<VkDeviceQueueCreateInfo> queue_infos,
		    VkPhysicalDeviceFeatures req_features,
		    std::vector<const char *> req_extensions,
		    VkDevice* device);

	// Generates a vector of VkDeviceQueueCreateInfo from a list of queue
	// families that can be used to create a device.
        template <class InputIt>
        std::vector<VkDeviceQueueCreateInfo> default_queue_infos(InputIt start, InputIt stop) {
		std::vector<VkDeviceQueueCreateInfo> queue_infos;

		for (; start != stop; ++start) {
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueCount = 1;
			info.queueFamilyIndex = *start;
			info.pQueuePriorities = &DEFAULT_QUEUE_PRIORITY;
			queue_infos.push_back(info);
		}

		return queue_infos;
	}
}

#endif // VK_DEVICE_H
