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
const bool VALIDATION_LAYERS_ON = true;
#else
const bool VALIDATION_LAYERS_ON = false;
#endif

struct GWindow {
	GLFWwindow* window;

	// Pointers are valid until GLFW terminates (so until the GWindow is
	// destroyed)
	uint32_t req_instance_ext_ct;
	const char** req_instance_exts;

	GWindow(uint32_t width, uint32_t height) {
		glfwInit();
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

		req_instance_exts = glfwGetRequiredInstanceExtensions(&req_instance_ext_ct);
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

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	// Create instance
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "My Thingy";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Custom Shenanigans";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstance instance;
	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;

	std::vector<const char*> extensions(glfw_window.req_instance_exts, glfw_window.req_instance_exts + glfw_window.req_instance_ext_ct);
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

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}
}
