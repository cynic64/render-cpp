#ifndef VK_BASE_H
#define VK_BASE_H

#include <vulkan/vulkan.h>
#include <tuple>

namespace vk_base {
	struct Base;

	// Creates the basics without any extensions or referencing a surface
	struct Default {
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base);

		VkPhysicalDevice create_phys_dev(const Base& base);
	};

	struct Base {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_msgr;

		template <typename D = Default, typename... Ts>
		Base() : instance(VK_NULL_HANDLE), debug_msgr(VK_NULL_HANDLE) {
			D deps;
			
			std::tie(instance, debug_msgr) = deps.create_instance(*this);
			deps.create_phys_dev(*this);
		}
	};
}

#endif // VK_BASE_H
