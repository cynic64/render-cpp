#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_queue.hpp"
#include "vk_swapchain.hpp"
#include "vk_image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <algorithm>

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

	std::pair<int, int> get_dims() {
		std::pair<int, int> dims;
		glfwGetFramebufferSize(window, &dims.first, &dims.second);

		return dims;
	}

	~GWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	VkInstance instance;
	// Will only actually be set if validation is enabled
	VkDebugUtilsMessengerEXT debug_msgr{};
	vk_instance::create(glfw_window.req_instance_exts, {}, VALIDATION_ENABLED, &instance, &debug_msgr);

	// Create surface
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, glfw_window.window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("Could not create surface!");

	// Create physical device
	VkPhysicalDevice phys_dev;
	auto phys_dev_name = vk_phys_dev::create(instance, vk_phys_dev::default_scorer, &phys_dev);
	std::cout << "Using device: " << phys_dev_name << std::endl;

	// Find queue families
	vk_queue::QueueFamilies queue_fams(phys_dev, surface);
	if (!queue_fams.graphics.has_value() || !queue_fams.present.has_value())
		throw std::runtime_error("Unable to find necessary queue families!");

	// Create logical device
	VkDevice device;

	// Necessary because the device has to support all our different queue
	// families
	auto dev_queue_infos = vk_device::default_queue_infos(queue_fams.unique.begin(), queue_fams.unique.end());

	std::vector<const char *> dev_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	vk_device::create(phys_dev, dev_queue_infos, {}, dev_extensions, &device);

	// Create swapchain
	auto [wwidth, wheight] = glfw_window.get_dims();
	auto swapchain = vk_swapchain::Swapchain(phys_dev, device, surface,
						 VK_NULL_HANDLE, queue_fams.unique.size(), queue_fams.unique.data(),
						 wwidth, wheight);

	// Choose swapchain settings
	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	swapchain.destroy();

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_msgr) vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
