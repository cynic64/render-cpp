#ifndef VK_BASE_H
#define VK_BASE_H

#include "vk_instance.hpp"
#include "vk_queue.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <tuple>

namespace vk_base {
	struct Base;

	// Creates the basics, does not create a surface or a present
	// queue. Constructor requires instance and device extensions.
	struct Default {
		Default(std::vector<const char *> instance_exts, std::vector<const char *> device_exts);
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base);
		VkSurfaceKHR create_surface(const Base& base);
		std::pair<VkPhysicalDevice, std::string> create_phys_dev(const Base& base);
		vk_queue::QueueFamilies create_queue_fams(const Base& base);
		VkDevice create_device(const Base& base);
		vk_queue::Queues create_queues(const Base& base);
	private:
		std::vector<const char *> instance_exts;
		std::vector<const char *> device_exts;
	};

	// Does everything Default does and also creates a surface from a GLFW window.
	struct Glfw : Default {
		Glfw(std::vector<const char *> instance_exts, std::vector<const char *> device_exts,
		     GLFWwindow* window);
		VkSurfaceKHR create_surface(const Base& base);
		vk_queue::QueueFamilies create_queue_fams(const Base& base);
	private:
		GLFWwindow* window;
	};

	struct Base {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_msgr;
		VkSurfaceKHR surface;
		VkPhysicalDevice phys_dev;
		std::string phys_dev_name;
		vk_queue::QueueFamilies queue_fams;
		VkDevice device;
		vk_queue::Queues queues;

		~Base() {
			if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance, surface, nullptr);

			vkDestroyDevice(device, nullptr);

			if (debug_msgr != VK_NULL_HANDLE)
				vk_instance::destroy_debug_msgr(instance, debug_msgr);
			vkDestroyInstance(instance, nullptr);
		}
	};

	template <typename D = Default, typename... Ts>
	Base create(Ts... args) {
		D deps(args...);
		Base b{};
			
		std::tie(b.instance, b.debug_msgr) = deps.create_instance(b);
		b.surface = deps.create_surface(b);
		std::tie(b.phys_dev, b.phys_dev_name) = deps.create_phys_dev(b);
		b.queue_fams = deps.create_queue_fams(b);
		b.device = deps.create_device(b);
		b.queues = deps.create_queues(b);

		return b;
	}
}

#endif // VK_BASE_H
