#ifndef VK_BASE_H
#define VK_BASE_H

#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace vk_base {
	struct Dependencies {
		std::pair<VkInstance, VkDebugUtilsMessengerEXT> create_instance();
	};

	template <typename D = Dependencies> class VkBase {
	public:
		template <typename... Ts>
		VkBase(Ts... args) : deps(D(args...)) {
			deps.create_instance();
		}

	private:
		D deps;
	};
}

#endif // VK_BASE_H
