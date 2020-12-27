#include "vk_instance.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

#ifdef NDEBUG
const bool VALIDATION_ENABLED = false;
#else
const bool VALIDATION_ENABLED = true;
#endif

struct GWindow {
	GLFWwindow* window;

	// Pointers are valid until GLFW terminates (so until the GWindow is
	// destroyed)
	std::vector<const char*> req_instance_exts;

	GWindow(uint32_t width, uint32_t height) {
		glfwInit();
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

		const char** raw_extensions;
		uint32_t extension_ct;
		raw_extensions = glfwGetRequiredInstanceExtensions(&extension_ct);
		req_instance_exts = {raw_extensions, raw_extensions + extension_ct};
	}

	~GWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	VkInstance instance;
	// Will only actually be filled if validation is enabled
	VkDebugUtilsMessengerEXT debug_msgr;
	vk_instance::create(glfw_window.req_instance_exts, {}, VALIDATION_ENABLED, &instance, &debug_msgr);

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
