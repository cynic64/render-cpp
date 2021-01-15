#ifndef VK_BASE_H
#define VK_BASE_H

#include "ll/instance.hpp"
#include "ll/queue.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <tuple>
#include <memory>

namespace base {
	struct Base;

	struct Dependencies {
		virtual std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base) = 0;
		virtual VkSurfaceKHR create_surface(const Base& base) = 0;
		virtual std::pair<VkPhysicalDevice, std::string> create_phys_dev(const Base& base) = 0;
		virtual ll::queue::QueueFamilies create_queue_fams(const Base& base) = 0;
		virtual VkDevice create_device(const Base& base) = 0;
		virtual ll::queue::Queues create_queues(const Base& base) = 0;
		virtual ~Dependencies() = default;
	};

	struct Base {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_msgr;
		VkSurfaceKHR surface;
		VkPhysicalDevice phys_dev;
		std::string phys_dev_name;
		ll::queue::QueueFamilies queue_fams;
		VkDevice device;
		ll::queue::Queues queues;

		Base(std::unique_ptr<Dependencies>&& deps);

		~Base();
	};

	// Creates the basics, does not create a surface or a present
	// queue. Constructor requires instance and device extensions.
	struct Default : Dependencies {
		Default(std::vector<const char *> instance_exts, std::vector<const char *> device_exts);
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance(const Base& base);
		VkSurfaceKHR create_surface(const Base& base);
		std::pair<VkPhysicalDevice, std::string> create_phys_dev(const Base& base);
		ll::queue::QueueFamilies create_queue_fams(const Base& base);
		VkDevice create_device(const Base& base);
		ll::queue::Queues create_queues(const Base& base);
	private:
		std::vector<const char *> instance_exts;
		std::vector<const char *> device_exts;
	};

	// Does everything Default does and also creates a surface from a GLFW window.
	struct Glfw : Default {
		Glfw(std::vector<const char *> instance_exts, std::vector<const char *> device_exts,
		     GLFWwindow* window);
		VkSurfaceKHR create_surface(const Base& base);
		ll::queue::QueueFamilies create_queue_fams(const Base& base);
	private:
		GLFWwindow* window;
	};

}

#endif // VK_BASE_H
