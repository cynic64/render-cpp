#include "../src/base.hpp"
#include "../src/glfw_window.hpp"

#include <iostream>
#include <string>
#include <memory>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

int main() {
	auto window = glfw_window::GWindow(INIT_WIDTH, INIT_HEIGHT);
	// C++ is truly a beautiful language
	base::Base b(std::make_unique<base::Glfw>(window.req_instance_exts,
						  std::vector<const char *>{VK_KHR_SWAPCHAIN_EXTENSION_NAME},
						  window.window));
	std::cout << "Using device: " << b.phys_dev_name << std::endl;
	std::cout << "Logical device: " << b.device << std::endl;
	std::cout << "Surface: " << b.surface << std::endl;
	std::cout << "Graphics queue: " << b.queues.graphics << std::endl;
	std::cout << "Present queue: " << b.queues.present << std::endl;
}
