#ifndef LL_PHYS_DEV_H
#define LL_PHYS_DEV_H

#include <vulkan/vulkan.h>
#include <string>

namespace ll::phys_dev {
	using ScoringFunction = auto (VkPhysicalDevice const&,
				      VkPhysicalDeviceProperties const&, VkPhysicalDeviceFeatures const&) -> int;

	// Returns the name of the chosen device and outputs to chosen_phys_dev
	auto create(VkInstance instance, ScoringFunction fun, VkPhysicalDevice* chosen_phys_dev) -> std::string;

	auto default_scorer(VkPhysicalDevice const& phys_dev,
			    VkPhysicalDeviceProperties const& props, VkPhysicalDeviceFeatures const&) -> int;
}

#endif // LL_PHYS_DEV_H
