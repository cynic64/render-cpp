#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_queue.hpp"

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

	// Choose swapchain settings
	VkSurfaceCapabilitiesKHR surface_caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &surface_caps);

	uint32_t format_ct;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(format_ct);
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &format_ct, formats.data());

	uint32_t present_mode_ct;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_ct);
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &present_mode_ct, present_modes.data());

	if (format_ct == 0 || present_mode_ct == 0)
		throw std::runtime_error("Surface formats or present modes unsupported by physical device!");

	VkFormat chosen_format;
	VkColorSpaceKHR chosen_color_space;
	VkPresentModeKHR chosen_present_mode;
	if (std::any_of(formats.begin(), formats.end(), [](auto f){return f.format == VK_FORMAT_B8G8R8A8_SRGB;}))
		chosen_format = VK_FORMAT_B8G8R8A8_SRGB;
	else chosen_format = formats[0].format;

	if (std::any_of(formats.begin(), formats.end(), [](auto f){return f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;}))
		chosen_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	else chosen_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	if (std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end())
		chosen_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	else chosen_present_mode = present_modes[0];

	uint32_t chosen_image_ct = std::min(surface_caps.minImageCount + 1, surface_caps.maxImageCount);

	auto [width, height] = glfw_window.get_dims();
	
	// Create swapchain
	VkSwapchainCreateInfoKHR swapchain_info{};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = surface;
	swapchain_info.minImageCount = chosen_image_ct;
	swapchain_info.imageFormat = chosen_format;
	swapchain_info.imageColorSpace = chosen_color_space;
	swapchain_info.imageExtent.width = std::clamp(static_cast<uint32_t>(width), surface_caps.minImageExtent.width, surface_caps.minImageExtent.width);
	swapchain_info.imageExtent.height = std::clamp(static_cast<uint32_t>(height), surface_caps.minImageExtent.height, surface_caps.minImageExtent.height);
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (queue_fams.unique.size() > 1) {
		swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_info.queueFamilyIndexCount = queue_fams.unique.size();
		swapchain_info.pQueueFamilyIndices = queue_fams.unique.data();
	} else swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = surface_caps.currentTransform;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode = chosen_present_mode;
	swapchain_info.clipped = VK_TRUE;

	VkSwapchainKHR swapchain;
	if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("Could not create swapchain!");

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_msgr) vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
