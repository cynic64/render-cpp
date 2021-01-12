#include "vk_base.hpp"

#include <iostream>
#include <string>
#include <memory>

struct Dependencies : vk_base::Dependencies {
	Dependencies(std::string message) : message(message) {}

	std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance() {
		std::cout << message << std::endl;

		return {};
	}

	std::string message;
};

int main() {
	vk_base::VkBase base;
	vk_base::VkBase<Dependencies> base2("hello");
}
