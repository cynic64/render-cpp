#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <algorithm>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk_device {
	// req_features is currently unimplemented
	void create(VkPhysicalDevice phys_dev,
		    std::vector<VkDeviceQueueCreateInfo> queue_infos,
		    VkPhysicalDeviceFeatures req_features,
		    std::vector<const char *> req_extensions,
		    VkDevice* device);
}

#endif // VK_DEVICE_H
