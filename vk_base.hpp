#ifndef VK_BASE_H
#define VK_BASE_H

#include <vulkan/vulkan.h>
#include <tuple>

namespace vk_base {
	struct Base {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_msgr;
	};

	// Creates the basics without any extensions or referencing a surface
	struct Default {
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base);

		VkPhysicalDevice create_phys_dev(const Base& base);
	};

	template <typename D = Default, typename... Ts>
	Base create_base(Ts... args) {
		D deps(args...);
		Base base{};

		std::tie(base.instance, base.debug_msgr) = deps.create_instance(base);
		deps.create_phys_dev(base);

		return base;
	}
}

#endif // VK_BASE_H
