#include "vk_base.hpp"
#include "glfw_window.hpp"

#include <iostream>
#include <string>
#include <memory>

int main() {
	auto window = glfw_window::GWindow(800, 600);
	auto b = vk_base::create<vk_base::Glfw>(window.req_instance_exts, std::vector<const char *>{},
						window.window);
	std::cout << "Using device: " << b.phys_dev_name << std::endl;
	std::cout << "Logical device: " << b.device << std::endl;
	std::cout << "Surface: " << b.surface << std::endl;
}
