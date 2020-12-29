#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace vk_instance {
	// Validation layers and extensions will only be enabled if the user
	// requests it.
	const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
	const std::vector<const char*> VALIDATION_EXTENSIONS = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

	bool check_validation_layer_support(std::vector<const char*> req_layers) {
		uint32_t supported_ct;
		vkEnumerateInstanceLayerProperties(&supported_ct, nullptr);

		std::vector<VkLayerProperties> supported(supported_ct);
		vkEnumerateInstanceLayerProperties(&supported_ct, supported.data());

		for (const char* layer : req_layers)
			if (std::none_of(supported.begin(), supported.end(),
					 [&](auto a){return strcmp(a.layerName, layer) == 0;}))
				return false;

		return true;
	}

	bool check_instance_ext_support(std::vector<const char*> req_exts) {
		uint32_t supported_ext_ct;
		vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_ct, nullptr);
		std::vector<VkExtensionProperties> supported(supported_ext_ct);
		vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_ct, supported.data());

		for (const char* extension : req_exts)
			if (std::none_of(supported.begin(), supported.end(),
					 [&](auto s){return strcmp(s.extensionName, extension) == 0;}))
				return false;

		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
							     VkDebugUtilsMessageTypeFlagsEXT,
							     const VkDebugUtilsMessengerCallbackDataEXT* data,
							     void*) {
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			std::cerr << "validation layer: " << data->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult create_debug_msgr(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info,
				   VkDebugUtilsMessengerEXT* msgr) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func) return func(instance, info, nullptr, msgr);
		else return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void destroy_debug_msgr(VkInstance instance, VkDebugUtilsMessengerEXT debug_msgr) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func) func(instance, debug_msgr, nullptr);
		else throw std::runtime_error("Coud not get address of vkDestroyDebugUtilsMessengerEXT!");
	}

	// If validation is enabled, VALIDATION_EXTENSIONS and VALIDATION_LAYERS
	// will be added to the user-specified extensions and layers, and a
	// debug callback will be added. debug_msgr maybe be null if validation
	// is not enabled.
	void create(std::vector<const char*> extensions,
		    std::vector<const char*> layers,
		    bool validation_enabled,
		    VkInstance* instance, VkDebugUtilsMessengerEXT* debug_msgr) {
		VkInstanceCreateInfo instance_info{};
		instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "My Thingy";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Custom Shenanigans";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;
		instance_info.pApplicationInfo = &app_info;

		VkDebugUtilsMessengerCreateInfoEXT debug_msgr_info{};
		debug_msgr_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_msgr_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_msgr_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_msgr_info.pfnUserCallback = debug_callback;

		if (validation_enabled) {
			extensions.insert(extensions.end(), VALIDATION_EXTENSIONS.begin(), VALIDATION_EXTENSIONS.end());
			layers.insert(layers.end(), VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());

			// Will create a debug messenger during instance
			// creation and destruction
			instance_info.pNext = &debug_msgr_info;
		}
		if (!check_instance_ext_support(extensions))
			throw std::runtime_error("Not all required instance extensions supported!");
		if (!check_validation_layer_support(layers))
			throw std::runtime_error("Validation layers turned on but not supported!");

		instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_info.ppEnabledExtensionNames = extensions.data();

		instance_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
		instance_info.ppEnabledLayerNames = layers.data();

		if(vkCreateInstance(&instance_info, nullptr, instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance!");

		if (validation_enabled) {
			// Create debug messenger
			if (create_debug_msgr(*instance, &debug_msgr_info, debug_msgr) != VK_SUCCESS)
				throw std::runtime_error("Failed to create debug messenger!");
		}

	}
}
