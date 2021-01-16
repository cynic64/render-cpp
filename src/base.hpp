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
		virtual auto create_instance(const Base&) -> std::pair<VkInstance, VkDebugUtilsMessengerEXT> = 0;
		virtual auto create_surface(const Base&) -> VkSurfaceKHR = 0;
		virtual auto create_phys_dev(const Base&) -> std::pair<VkPhysicalDevice, std::string> = 0;
		virtual auto create_queue_fams(const Base&) -> ll::queue::QueueFamilies = 0;
		virtual auto create_device(const Base&) -> VkDevice = 0;
		virtual auto create_queues(const Base&) -> ll::queue::Queues = 0;
		// Should this be virtual? Should this be default? Who the fuck knows.
		virtual ~Dependencies() = default;
	};

	struct Base {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_msgr = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice phys_dev = VK_NULL_HANDLE;
		std::string phys_dev_name;
		ll::queue::QueueFamilies queue_fams;
		VkDevice device = VK_NULL_HANDLE;
		ll::queue::Queues queues;

		Base(std::unique_ptr<Dependencies>&& deps);
		~Base();
	};

	// Creates the basics, does not create a surface or a present
	// queue. Constructor requires instance and device extensions.
	struct Default : Dependencies {
		Default(std::vector<const char *> instance_exts, std::vector<const char *> device_exts);
		auto create_instance(const Base& base) -> std::pair<VkInstance, VkDebugUtilsMessengerEXT> override;
		auto create_surface(const Base& base) -> VkSurfaceKHR override;
		auto create_phys_dev(const Base& base) -> std::pair<VkPhysicalDevice, std::string> override;
		auto create_queue_fams(const Base& base) -> ll::queue::QueueFamilies override;
		auto create_device(const Base& base) -> VkDevice override;
		auto create_queues(const Base& base) -> ll::queue::Queues override;
	private:
		std::vector<const char *> instance_exts;
		std::vector<const char *> device_exts;
	};

	// Does everything Default does and also creates a surface from a GLFW window.
	struct Glfw : Default {
		Glfw(std::vector<const char *> instance_exts, std::vector<const char *> device_exts,
		     GLFWwindow* window);
		auto create_surface(const Base& base) -> VkSurfaceKHR override;
		auto create_queue_fams(const Base& base) -> ll::queue::QueueFamilies override;
	private:
		GLFWwindow* window;
	};

}

#endif // VK_BASE_H
