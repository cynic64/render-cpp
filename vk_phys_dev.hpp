#ifndef VK_PHYS_DEV_H
#define VK_PHYS_DEV_H

#include <vulkan/vulkan.h>
#include <string>

namespace vk_phys_dev {
	typedef int (*ScoringFunction)(VkPhysicalDeviceProperties const&, VkPhysicalDeviceFeatures const&);

	// Returns the name of the chosen device and outputs to chosen_phys_dev
	std::string create(VkInstance instance, ScoringFunction fun, VkPhysicalDevice* chosen_phys_dev);

	int default_scorer(VkPhysicalDeviceProperties const& props, VkPhysicalDeviceFeatures const& features);
}

#endif // VK_PHYS_DEV_H
