#include "vk_base.hpp"

#include "vk_instance.hpp"

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

	VkPhysicalDevice Default::create_phys_dev(const Base& base) {
		std::cout << "Instance: " << base.instance << std::endl;
		return 0;
	}
}
