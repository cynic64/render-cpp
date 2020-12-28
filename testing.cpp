#include "vk_instance.hpp"
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>

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

	// Create physical device
	uint32_t phys_dev_ct;
	vkEnumeratePhysicalDevices(instance, &phys_dev_ct, nullptr);
	if (phys_dev_ct == 0) throw std::runtime_error("No GPUs found!");

	std::vector<VkPhysicalDevice> phys_devs(phys_dev_ct);
	vkEnumeratePhysicalDevices(instance, &phys_dev_ct, phys_devs.data());
	auto phys_dev = phys_devs[0];
	VkPhysicalDeviceProperties phys_dev_props;
	vkGetPhysicalDeviceProperties(phys_dev, &phys_dev_props);
	std::cout << "Using device: " << phys_dev_props.deviceName << std::endl;

	// Find queue family
	std::optional<uint32_t> graphics_fam;
	uint32_t queue_fam_ct;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct, nullptr);
	std::vector<VkQueueFamilyProperties> queue_fams(queue_fam_ct);
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct, queue_fams.data());

	for (size_t i = 0; i < queue_fam_ct; ++i) {
		if (queue_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphics_fam = i;
			break;
		}
	}

	if (!graphics_fam.has_value()) throw std::runtime_error("Could not find queue family with graphics support!");

	// Create logical device
	VkDevice device;

	VkDeviceQueueCreateInfo dev_queue_info{};
	dev_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dev_queue_info.queueCount = 1;
	dev_queue_info.queueFamilyIndex = graphics_fam.value();
	float queue_priority = 1.0f;
	dev_queue_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures dev_features{};

	VkDeviceCreateInfo device_info{};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &dev_queue_info;
	device_info.pEnabledFeatures = &dev_features;

	if (vkCreateDevice(phys_dev, &device_info, nullptr, &device) != VK_SUCCESS) throw std::runtime_error("Could not create device!");

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vkDestroyDevice(device, nullptr);

	vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
