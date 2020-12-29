#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>

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
	std::optional<uint32_t> graphics_fam;
	std::optional<uint32_t> present_fam;
	uint32_t queue_fam_ct;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct, nullptr);
	std::vector<VkQueueFamilyProperties> queue_fams(queue_fam_ct);
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct, queue_fams.data());

	for (uint32_t i = 0; i < queue_fam_ct; ++i) {
		if (queue_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphics_fam = i;
		VkBool32 present_supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, i, surface, &present_supported);
		if (present_supported) present_fam = i;

		if (graphics_fam.has_value() && present_fam.has_value()) break;
	}

	if (!graphics_fam.has_value() || !present_fam.has_value())
		throw std::runtime_error("Could not find queue families supporting graphics and presentation!");

	std::unordered_set<uint32_t> unique_queue_fams;
	unique_queue_fams.insert(graphics_fam.value());
	unique_queue_fams.insert(present_fam.value());

	// Create logical device
	VkDevice device;

	// Necessary because the device has to support all our different queue
	// families
	std::vector<VkDeviceQueueCreateInfo> dev_queue_infos;
	float queue_priority = 1.0f;
	for (auto queue_fam : unique_queue_fams) {
		VkDeviceQueueCreateInfo dev_queue_info{};
		dev_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		dev_queue_info.queueCount = 1;
		dev_queue_info.queueFamilyIndex = queue_fam;
		dev_queue_info.pQueuePriorities = &queue_priority;
		dev_queue_infos.push_back(dev_queue_info);
        }

        VkPhysicalDeviceFeatures dev_features{};

	VkDeviceCreateInfo device_info{};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = static_cast<uint32_t>(dev_queue_infos.size());
	device_info.pQueueCreateInfos = dev_queue_infos.data();
	device_info.pEnabledFeatures = &dev_features;

	if (vkCreateDevice(phys_dev, &device_info, nullptr, &device) != VK_SUCCESS) throw std::runtime_error("Could not create device!");

	// Create queues
	VkQueue graphics_queue, present_queue;
	vkGetDeviceQueue(device, graphics_fam.value(), 0, &graphics_queue);
	vkGetDeviceQueue(device, present_fam.value(), 0, &present_queue);

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_msgr) vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
