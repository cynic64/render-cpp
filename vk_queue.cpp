#include "vk_queue.hpp"

#include <vector>
#include <unordered_set>

namespace vk_queue {
	QueueFamilies::QueueFamilies(VkPhysicalDevice phys_dev, std::optional<VkSurfaceKHR> surface) {
		uint32_t queue_fam_ct;
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct,
							 nullptr);
		std::vector<VkQueueFamilyProperties> queue_fams(queue_fam_ct);
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_fam_ct,
							 queue_fams.data());
		
		for (uint32_t i = 0; i < queue_fam_ct; ++i) {
			if (queue_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphics = i;

                        if (surface.has_value()) {
				VkBool32 present_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(
					phys_dev, i, surface.value(), &present_supported);
				if (present_supported)
					present = i;
                        }
                        if (graphics.has_value() && present.has_value())
				break;
		}
		
		std::unordered_set<uint32_t> fam_set;
		if (graphics.has_value()) fam_set.insert(graphics.value());
		if (present.has_value()) fam_set.insert(present.value());

		unique = std::vector<uint32_t>(fam_set.begin(), fam_set.end());
        }

	Queues::Queues(VkDevice device, QueueFamilies queue_fams) {
		if (queue_fams.graphics.has_value())
			vkGetDeviceQueue(device, queue_fams.graphics.value(), 0, &graphics);
		if (queue_fams.present.has_value()) {
			if (queue_fams.present.value() == queue_fams.graphics.value())
				present = graphics;
			else vkGetDeviceQueue(device, queue_fams.present.value(), 0, &present);
		}
	}
}
