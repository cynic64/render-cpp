#include "vk_base.hpp"

#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_device.hpp"

#include <vulkan/vulkan.h>
#include <iostream>

namespace vk_base {

#ifdef NDEBUG
	const bool VALIDATION_ENABLED = false;
#else
	const bool VALIDATION_ENABLED = true;
#endif

	/*
	 * Default
	 */
	Default::Default(std::vector<const char *> instance_exts, std::vector<const char *> device_exts)
		: instance_exts(instance_exts), device_exts(device_exts) {}

	std::pair<VkInstance, VkDebugUtilsMessengerEXT> Default::create_instance(const Base&) {
		// DebugUtils... will only actually be set if validation is enabled
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> out;
		vk_instance::create(instance_exts, {}, VALIDATION_ENABLED, &out.first, &out.second);

		return out;
	}

	VkSurfaceKHR Default::create_surface(const Base&) { return VK_NULL_HANDLE; }

	std::pair<VkPhysicalDevice, std::string> Default::create_phys_dev(const Base& base) {
		std::pair<VkPhysicalDevice, std::string> out;
		out.second = vk_phys_dev::create(base.instance, vk_phys_dev::default_scorer, &out.first);

		return out;
	}

	vk_queue::QueueFamilies Default::create_queue_fams(const Base& base) {
		vk_queue::QueueFamilies queue_fams(base.phys_dev);
		if (!queue_fams.graphics.has_value())
			throw std::runtime_error("Unable to find graphics family!");

		return queue_fams;
	}

	// Does not enable any extensions.
	VkDevice Default::create_device(const Base& base) {
		// Necessary because the device has to support all our different queue
		// families
		auto dev_queue_infos = vk_device::default_queue_infos(base.queue_fams.unique.begin(),
								      base.queue_fams.unique.end());

		VkDevice device;
		vk_device::create(base.phys_dev, dev_queue_infos, {}, device_exts, &device);

		return device;
	}

	/*
	 * GLFW
	 */
	Glfw::Glfw(std::vector<const char *> instance_exts, std::vector<const char *> device_exts,
		   GLFWwindow* window) : Default(instance_exts, device_exts), window(window) {}

	VkSurfaceKHR Glfw::create_surface(const Base &base) {
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(base.instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("Could not create surface!");
		return surface;
	}
}
