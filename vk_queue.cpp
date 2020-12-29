#include "vk_queue.hpp"

#include <vector>

namespace vk_queue {
	QueueFamilies::QueueFamilies(VkPhysicalDevice phys_dev, VkSurfaceKHR surface) {
		uint32_t queue_fam_ct;
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct,
							 nullptr);
		std::vector<VkQueueFamilyProperties> queue_fams(queue_fam_ct);
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct,
							 queue_fams.data());
		
		for (uint32_t i = 0; i < queue_fam_ct; ++i) {
			if (queue_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphics = i;

                        if (surface) {
				VkBool32 present_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(
					phys_dev, i, surface, &present_supported);
				if (present_supported)
					present = i;
                        }
                        if (graphics.has_value() && present.has_value())
				break;
		}
		
		unique.insert(graphics.value());
		unique.insert(present.value());
        }
}
