#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <stdexcept>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool VALIDATION_LAYERS_ON = false;
#else
const bool VALIDATION_LAYERS_ON = true;
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

bool check_validation_layer_support(std::vector<const char*> req_layers) {
	uint32_t supported_ct;
	vkEnumerateInstanceLayerProperties(&supported_ct, nullptr);

	std::vector<VkLayerProperties> supported(supported_ct);
	vkEnumerateInstanceLayerProperties(&supported_ct, supported.data());

	for (const char* layer : req_layers) {
		if (std::none_of(supported.begin(), supported.end(),
				 [&](auto a){return strcmp(a.layerName, layer) == 0;}))
			return false;
	}

	return true;
}

bool check_instance_ext_support(std::vector<const char*> req_exts) {
	uint32_t supported_ext_ct;
	vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_ct, nullptr);
	std::vector<VkExtensionProperties> supported_exts(supported_ext_ct);
	vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_ct, supported_exts.data());

	// What a beautiful, concise language C++ is...
	return std::all_of(req_exts.begin(), req_exts.end(), [&](auto e){
		return std::any_of(supported_exts.begin(), supported_exts.end(),
				   [&](auto s){return strcmp(e, s.extensionName) == 0;});
	});
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

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	// Create instance
	VkInstance instance;

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "My Thingy";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Custom Shenanigans";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_info{};

	VkDebugUtilsMessengerCreateInfoEXT debug_msgr_info{};
	debug_msgr_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_msgr_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_msgr_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_msgr_info.pfnUserCallback = debug_callback;

	if (VALIDATION_LAYERS_ON) instance_info.pNext = &debug_msgr_info;

	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;

	auto extensions = glfw_window.req_instance_exts;
	if (VALIDATION_LAYERS_ON) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	if (!check_instance_ext_support(extensions))
		throw std::runtime_error("Not all required instance extensions supported!");

	instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_info.ppEnabledExtensionNames = extensions.data();

	if (VALIDATION_LAYERS_ON) {
		if (!check_validation_layer_support(VALIDATION_LAYERS))
			throw std::runtime_error("Validation layers turned on but not supported!");

		instance_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		instance_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}

	if(vkCreateInstance(&instance_info, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");

	if (VALIDATION_LAYERS_ON) {
		// Create debug messenger
		VkDebugUtilsMessengerEXT debug_msgr;
		if (create_debug_msgr(instance, &debug_msgr_info, &debug_msgr) != VK_SUCCESS)
			throw std::runtime_error("Failed to create debug messenger!");
	}

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vkDestroyInstance(instance, nullptr);
}
