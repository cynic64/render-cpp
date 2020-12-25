#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

struct GWindow {
	GLFWwindow* window;

	// Pointers are valid until GLFW terminates (so until the GWindow is
	// destroyed)
	uint32_t req_instance_ext_ct;
	const char** req_instance_exts;

	GWindow(uint32_t width, uint32_t height) {
		glfwInit();
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

		req_instance_exts = glfwGetRequiredInstanceExtensions(&req_instance_ext_ct);
	}

	~GWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	// Create instance
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "My Thingy";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Custom Shenanigans";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstance instance;
	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = glfw_window.req_instance_ext_ct;
	instance_info.ppEnabledExtensionNames = glfw_window.req_instance_exts;
	VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}
}
