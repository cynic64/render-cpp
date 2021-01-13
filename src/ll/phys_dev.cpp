#include "phys_dev.hpp"

#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace ll::phys_dev {
	std::string create(VkInstance instance, ScoringFunction fun, VkPhysicalDevice* chosen_phys_dev) {
		uint32_t phys_dev_ct;
		vkEnumeratePhysicalDevices(instance, &phys_dev_ct, nullptr);
		if (phys_dev_ct == 0)
			throw std::runtime_error("No GPUs found!");
		
		std::vector<VkPhysicalDevice> phys_devs(phys_dev_ct);
		vkEnumeratePhysicalDevices(instance, &phys_dev_ct, phys_devs.data());

		auto best_score = 0;
		std::string best_name;
		for (auto const& phys_dev : phys_devs) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(phys_dev, &props);
			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(phys_dev, &features);

			auto score = fun(phys_dev, props, features);
			if (score > best_score) {
				best_score = score;
				*chosen_phys_dev = phys_dev;
				best_name = props.deviceName;
			}
		}

		if (!best_score) throw std::runtime_error("No GPU received a postive score!");

		return best_name;
	}
	
	int default_scorer(VkPhysicalDevice const&,
			   VkPhysicalDeviceProperties const& props, VkPhysicalDeviceFeatures const&) {
		auto score = 1;
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1;
		
		return score;
	}
}
