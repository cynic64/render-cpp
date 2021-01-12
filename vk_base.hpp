#ifndef VK_BASE_H
#define VK_BASE_H

#include "vk_queue.hpp"

#include <vulkan/vulkan.h>
#include <tuple>

namespace vk_base {
	struct Base;

	// Creates the basics without any extensions or referencing a surface
	struct Default {
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base);
		std::pair<VkPhysicalDevice, std::string> create_phys_dev(const Base& base);
		vk_queue::QueueFamilies create_queue_fams(const Base& base);
	};

	struct Base {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_msgr;
		VkPhysicalDevice phys_dev;
		std::string phys_dev_name;
		vk_queue::QueueFamilies queue_fams;

		template <typename D = Default, typename... Ts>
		Base(Ts... args) : instance(VK_NULL_HANDLE), debug_msgr(VK_NULL_HANDLE),
				   phys_dev(VK_NULL_HANDLE) {
			D deps(args...);
			
			std::tie(instance, debug_msgr) = deps.create_instance(*this);
			std::tie(phys_dev, phys_dev_name) = deps.create_phys_dev(*this);
			queue_fams = deps.create_queue_fams(*this);
		}
	};
}

#endif // VK_BASE_H
