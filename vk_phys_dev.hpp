#ifndef VK_PHYS_DEV_H
#define VK_PHYS_DEV_H

#include <vulkan/vulkan.h>
#include <string>

namespace vk_phys_dev {
	typedef int (*ScoringFunction)(VkPhysicalDevice const&, VkPhysicalDeviceProperties const&, VkPhysicalDeviceFeatures const&);

	// Returns the name of the chosen device and outputs to chosen_phys_dev
	std::string create(VkInstance instance, ScoringFunction fun, VkPhysicalDevice* chosen_phys_dev);

	int default_scorer(VkPhysicalDevice const& phys_dev, VkPhysicalDeviceProperties const& props, VkPhysicalDeviceFeatures const&);
}

#endif // VK_PHYS_DEV_H
