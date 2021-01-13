#include <vulkan/vulkan.h>
#ifndef LL_INSTANCE_H
#define LL_INSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace ll::instance {
	// If validation is enabled, VALIDATION_EXTENSIONS and VALIDATION_LAYERS
	// will be added to the user-specified extensions and layers, and a
	// debug callback will be added. debug_msgr maybe be null if validation
	// is not enabled.
	void create(std::vector<const char*> extensions,
		    std::vector<const char*> layers,
		    bool validation_enabled,
		    VkInstance* instance, VkDebugUtilsMessengerEXT* debug_msgr);

	void destroy_debug_msgr(VkInstance instance, VkDebugUtilsMessengerEXT debug_msgr);
}

#endif // LL_INSTANCE_H
