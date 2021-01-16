#include "loop.hpp"

namespace loop {
	Loop::Loop(std::unique_ptr<Dependencies>&& deps, VkDevice device, ll::queue::Queues queues)
		: device(device), queues(queues)
	{
		swapchain = deps->create_swapchain(std::as_const(*this));
	}
}
