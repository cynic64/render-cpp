#include "vk_base.hpp"

#include <iostream>

namespace vk_base {
	std::pair<VkInstance, VkDebugUtilsMessengerEXT> Dependencies::create_instance() {
		std::cout << "Hello from default implementation!" << std::endl;

		return {};
	}
}
