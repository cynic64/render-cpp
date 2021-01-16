#ifndef LOOP_H
#define LOOP_H

#include "ll/swapchain.hpp"
#include "ll/queue.hpp"

#include <vulkan/vulkan.h>
#include <memory>

namespace loop {
	struct Loop;

	struct Dependencies {
		virtual auto create_swapchain(const Loop&) -> ll::swapchain::Swapchain = 0;
	};

	struct Loop {
		VkDevice device = VK_NULL_HANDLE;
		ll::queue::Queues queues;
		ll::swapchain::Swapchain swapchain;
		std::vector<VkFence> image_fences;
		std::vector<VkSemaphore> image_avail_sems;
		std::vector<VkSemaphore> render_done_sems;

		Loop(std::unique_ptr<Dependencies>&& deps, VkDevice device, ll::queue::Queues queues);
		void draw();

	private:
		std::unique_ptr<Dependencies> deps;
	};

        /*
         * maybe recreate
         * wait for cbuf's RDF
         * acquire image
         * wait for image's RDF
         * mark acquired image with RDF
	 * record
         * submit
	 * present, set must_recreate
	 */
}

#endif // LOOP_H
