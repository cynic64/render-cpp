#include "base.hpp"

#include "ll/instance.hpp"
#include "ll/phys_dev.hpp"
#include "ll/device.hpp"

#include <vulkan/vulkan.h>
#include <iostream>

namespace base {

#ifdef NDEBUG
	const bool VALIDATION_ENABLED = false;
#else
	const bool VALIDATION_ENABLED = true;
#endif
	/*
	 * Base
	 */
	Base::Base(std::unique_ptr<Dependencies>&& deps) {
		std::tie(instance, debug_msgr) = deps->create_instance(*this);
		surface = deps->create_surface(*this);
		std::tie(phys_dev, phys_dev_name) = deps->create_phys_dev(*this);
		queue_fams = deps->create_queue_fams(*this);
		device = deps->create_device(*this);
		queues = deps->create_queues(*this);
	}

	Base::~Base() {
		if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance, surface, nullptr);

		vkDestroyDevice(device, nullptr);

		if (debug_msgr != VK_NULL_HANDLE)
			ll::instance::destroy_debug_msgr(instance, debug_msgr);
		vkDestroyInstance(instance, nullptr);
	}

	/*
	 * Default
	 */
	Default::Default(std::vector<const char *> instance_exts, std::vector<const char *> device_exts)
		: instance_exts(instance_exts), device_exts(device_exts) {}

	std::pair<VkInstance, VkDebugUtilsMessengerEXT> Default::create_instance(const Base&) {
		// DebugUtils... will only actually be set if validation is enabled
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> out;
		ll::instance::create(instance_exts, {}, VALIDATION_ENABLED, &out.first, &out.second);

		return out;
	}

	VkSurfaceKHR Default::create_surface(const Base&) { return VK_NULL_HANDLE; }

	std::pair<VkPhysicalDevice, std::string> Default::create_phys_dev(const Base& base) {
		std::pair<VkPhysicalDevice, std::string> out;
		out.second = ll::phys_dev::create(base.instance, ll::phys_dev::default_scorer, &out.first);

		return out;
	}

	ll::queue::QueueFamilies Default::create_queue_fams(const Base& base) {
		ll::queue::QueueFamilies queue_fams;
		if (base.surface != VK_NULL_HANDLE) queue_fams = {base.phys_dev, base.surface};
		else queue_fams = {base.phys_dev};

		if (!queue_fams.graphics.has_value())
			throw std::runtime_error("Unable to find graphics family!");

		return queue_fams;
	}

	VkDevice Default::create_device(const Base& base) {
		// The device has to support all our different queue families
		auto dev_queue_infos = ll::device::default_queue_infos(base.queue_fams.unique.begin(),
								      base.queue_fams.unique.end());

		VkDevice device;
		ll::device::create(base.phys_dev, dev_queue_infos, {}, device_exts, &device);

		return device;
	}

	ll::queue::Queues Default::create_queues(const Base& base) {
		return ll::queue::Queues(base.device, base.queue_fams);
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

	ll::queue::QueueFamilies Glfw::create_queue_fams(const Base& base) {
		auto queues =  Default::create_queue_fams(base);
		if (!queues.present.has_value())
			throw std::runtime_error("No present queue!");

		return queues;
	}
}
