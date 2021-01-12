#include "vk_base.hpp"

#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"

#include <vulkan/vulkan.h>
#include <iostream>

namespace vk_base {

#ifdef NDEBUG
	const bool VALIDATION_ENABLED = false;
#else
	const bool VALIDATION_ENABLED = true;
#endif

	std::pair<VkInstance, VkDebugUtilsMessengerEXT> Default::create_instance(const Base&) {
		// DebugUtils... will only actually be set if validation is enabled

		std::pair<VkInstance, VkDebugUtilsMessengerEXT> out;
		vk_instance::create({}, {}, VALIDATION_ENABLED, &out.first, &out.second);

		return out;
	}

	std::pair<VkPhysicalDevice, std::string> Default::create_phys_dev(const Base& base) {
		std::pair<VkPhysicalDevice, std::string> out;
		out.second = vk_phys_dev::create(base.instance, vk_phys_dev::default_scorer, &out.first);

		return out;
	}

	vk_queue::QueueFamilies Default::create_queue_fams(const Base& base) {
		vk_queue::QueueFamilies queue_fams(base.phys_dev);
		if (!queue_fams.graphics.has_value())
			throw std::runtime_error("Unable to find graphics family!");

		return queue_fams;
	}
}
