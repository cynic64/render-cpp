#include "device.hpp"
#include <stdexcept>
#include <cstring>

namespace ll::device {
	bool check_extension_support(VkPhysicalDevice phys_dev, std::vector<const char *> req_extensions) {
		uint32_t ext_ct;
		vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_ct, nullptr);

		std::vector<VkExtensionProperties> supported(ext_ct);
		vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_ct, supported.data());

		for (auto const& ext : req_extensions) {
			if (std::none_of(supported.begin(), supported.end(),
					 [&](auto a){return strcmp(a.extensionName, ext) == 0;}))
				return false;
		}

		return true;
	}

	void create(VkPhysicalDevice phys_dev,
		    std::vector<VkDeviceQueueCreateInfo> queue_infos,
		    VkPhysicalDeviceFeatures req_features,
		    std::vector<const char *> req_extensions,
		    VkDevice* device) {
		if (!check_extension_support(phys_dev, req_extensions))
			throw std::runtime_error("Required device extensions not supported!");

		VkDeviceCreateInfo device_info{};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
		device_info.pQueueCreateInfos = queue_infos.data();
		device_info.pEnabledFeatures = &req_features;
		device_info.enabledExtensionCount = static_cast<uint32_t>(req_extensions.size());
		device_info.ppEnabledExtensionNames = req_extensions.data();

		if (vkCreateDevice(phys_dev, &device_info, nullptr, device) != VK_SUCCESS)
			throw std::runtime_error("Could not create device!");
	}
}
