#include "../src/base.hpp"
#include "../src/loop.hpp"
#include "../src/ll/device.hpp"
#include "../src/ll/instance.hpp"
#include "../src/ll/phys_dev.hpp"
#include "../src/ll/queue.hpp"
#include "../src/ll/swapchain.hpp"
#include "../src/ll/image.hpp"
#include "../src/ll/shader.hpp"
#include "../src/ll/rpass.hpp"
#include "../src/ll/pipeline.hpp"
#include "../src/ll/cbuf.hpp"
#include "../src/ll/sync.hpp"
#include "../src/timer.hpp"
#include "../src/glfw_window.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <type_traits>
#include <chrono>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

const auto CBUF_CT = 4;

#ifdef NDEBUG
const bool VALIDATION_ENABLED = false;
#else
const bool VALIDATION_ENABLED = true;
#endif

bool must_recreate = false;

void resize_callback(GLFWwindow*, int, int) {
	must_recreate = true;
}

int main() {
	auto window = glfw_window::GWindow(INIT_WIDTH, INIT_HEIGHT);
	glfwSetFramebufferSizeCallback(window.window, resize_callback);

	base::Base base(std::make_unique<base::Glfw>(window.req_instance_exts,
						     std::vector<const char *>{VK_KHR_SWAPCHAIN_EXTENSION_NAME},
						     window.window));

	std::cout << "Using device: " << base.phys_dev_name << std::endl;

	// Load shaders
	auto vs = ll::shader::Shader(base.device, VK_SHADER_STAGE_VERTEX_BIT, "../shaders/shader.vert.spv");
	auto fs = ll::shader::Shader(base.device, VK_SHADER_STAGE_FRAGMENT_BIT, "../shaders/shader.frag.spv");

	VkPipelineShaderStageCreateInfo shaders[] = {vs.info, fs.info};

	auto pipeline_lt = ll::pipeline::layout(base.device);

	// Allocate command buffer
	VkCommandPoolCreateInfo cpool_info{};
	cpool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpool_info.queueFamilyIndex = base.queue_fams.graphics.value();
	cpool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool cpool;
	if (vkCreateCommandPool(base.device, &cpool_info, nullptr, &cpool) != VK_SUCCESS)
		throw std::runtime_error("Could not create command pool!");

	VkCommandBufferAllocateInfo cbuf_info{};
	cbuf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbuf_info.commandPool = cpool;
	cbuf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbuf_info.commandBufferCount = CBUF_CT;

	std::vector<VkCommandBuffer> cbufs(CBUF_CT);
	if (vkAllocateCommandBuffers(base.device, &cbuf_info, cbufs.data()) != VK_SUCCESS)
		throw std::runtime_error("Could not allocate command buffer!");

	// Dynamic state (other fields will be filled in later)
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.minDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};

	// Synchronization
	std::vector<VkSemaphore> image_avail_sems(CBUF_CT), render_done_sems(CBUF_CT);
	std::vector<VkFence> render_done_fences(CBUF_CT);

	for (size_t i = 0; i < CBUF_CT; ++i) {
		image_avail_sems[i] = ll::sync::semaphore(base.device);
		render_done_sems[i] = ll::sync::semaphore(base.device);
		render_done_fences[i] = ll::sync::fence(base.device, VK_FENCE_CREATE_SIGNALED_BIT);
	}

	// Everything from here on depends on the swapchain, so we make them
	// null for now
	ll::swapchain::Swapchain swapchain;
	VkRenderPass rpass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> fbs;
	std::vector<VkFence> image_fences;

	auto sync_set_idx = 0;

	// Main loop
	timer::Timer timer;
	size_t frame_ct = 0;
	must_recreate = true;

	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();

		while (must_recreate) {
			vkDeviceWaitIdle(base.device);
			// Clean up old stuff
			if (swapchain.swapchain != VK_NULL_HANDLE) swapchain.destroy();

			// Create swapchain
			auto [wwidth, wheight] = window.get_dims();
			swapchain = ll::swapchain::Swapchain(base.phys_dev, base.device, base.surface,
							    VK_NULL_HANDLE,
							    base.queue_fams.unique.size(), base.queue_fams.unique.data(),
							    wwidth, wheight);
			auto [newwidth, newwheight] = window.get_dims();
			if (newwidth != wwidth || newwheight != wheight) continue;

			if (rpass != VK_NULL_HANDLE) vkDestroyRenderPass(base.device, rpass, nullptr);
			if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(base.device, pipeline, nullptr);
			if (!fbs.empty()) for (auto& i : fbs) vkDestroyFramebuffer(base.device, i, nullptr);

			// Create render pass
			auto color_attachment = ll::rpass::attachment(swapchain.format);
			auto color_ref = ll::rpass::attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			auto subpass = ll::rpass::subpass(1, &color_ref);
			auto subpass_dep = ll::rpass::dependency();
			rpass = ll::rpass::rpass(base.device, 1, &color_attachment, 1, &subpass, 1, &subpass_dep);

			// Create pipeline
			pipeline = ll::pipeline::pipeline(base.device, 2, shaders, pipeline_lt, rpass);

			// Create framebuffers
			fbs.resize(swapchain.images.size());
			for (size_t i = 0; i < swapchain.images.size(); ++i) {
				auto view = swapchain.image_views[i];
				
				VkFramebufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				info.renderPass = rpass;
				info.attachmentCount = 1;
				info.pAttachments = &view;
				info.width = swapchain.width;
				info.height = swapchain.height;
				info.layers = 1;
				
				if (vkCreateFramebuffer(base.device, &info, nullptr, &fbs[i]) != VK_SUCCESS)
					throw std::runtime_error("Could not create framebuffer!");
			}

			// Clear image fences
			image_fences.clear();
			image_fences.resize(swapchain.images.size(), VK_NULL_HANDLE);

			// Recreate fences and semaphores
			for (size_t i = 0; i < CBUF_CT; ++i) {
				vkDestroySemaphore(base.device, image_avail_sems[i], nullptr);
				vkDestroySemaphore(base.device, render_done_sems[i], nullptr);
				vkDestroyFence(base.device, render_done_fences[i], nullptr);

				image_avail_sems[i] = ll::sync::semaphore(base.device);
				render_done_sems[i] = ll::sync::semaphore(base.device);
				render_done_fences[i] = ll::sync::fence(base.device, VK_FENCE_CREATE_SIGNALED_BIT);
			}


			// Update dynamic state
			viewport.width = swapchain.width;
			viewport.height = swapchain.width;
			scissor.extent.width = swapchain.width;
			scissor.extent.height = swapchain.width;

			must_recreate = false;
		}

		// Wait for the sync set we'll use to become available
		if (vkWaitForFences(base.device, 1, &render_done_fences[sync_set_idx], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			throw std::runtime_error("Could not wait for sync set's render-done fence!");

		auto cbuf = cbufs[sync_set_idx];
		vkResetCommandBuffer(cbuf, 0);

		uint32_t image_idx = 0;
		if (vkAcquireNextImageKHR(base.device, swapchain.swapchain, UINT64_MAX,
				      image_avail_sems[sync_set_idx], VK_NULL_HANDLE, &image_idx)
		    != VK_SUCCESS) {
			must_recreate = true;
			continue;
		}

		// Wait for whoever's drawing to our image to finish
		if (image_fences[image_idx] != VK_NULL_HANDLE)
			if (vkWaitForFences(base.device, 1, &image_fences[image_idx], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
				throw std::runtime_error("Could not wait for image's fence!");

		if (vkResetFences(base.device, 1, &render_done_fences[sync_set_idx]) != VK_SUCCESS)
			throw std::runtime_error("Could not reset render-done fence!");

		// We're now rendering to this image, so mark it with our fence
		image_fences[image_idx] = render_done_fences[sync_set_idx];

		ll::cbuf::begin(cbuf);
		ll::cbuf::begin_rpass(cbuf, rpass, fbs[image_idx], swapchain.width, swapchain.height);
		ll::cbuf::bind_pipeline(cbuf, pipeline);
		ll::cbuf::set_viewport(cbuf, {viewport});
		ll::cbuf::set_scissor(cbuf, {scissor});
		ll::cbuf::draw(cbuf, 3);
		ll::cbuf::end_rpass(cbuf);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_avail_sems[sync_set_idx];
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cbuf;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_done_sems[sync_set_idx];

		if (vkQueueSubmit(base.queues.graphics, 1, &submit_info, render_done_fences[sync_set_idx]) != VK_SUCCESS)
			throw std::runtime_error("Could not submit!");

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_done_sems[sync_set_idx];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain.swapchain;
		present_info.pImageIndices = &image_idx;

		auto res = vkQueuePresentKHR(base.queues.present, &present_info);
		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			must_recreate = true;
		} else if (res != VK_SUCCESS)
			throw std::runtime_error("Presenting failed with something other than out-of-date!");

		frame_ct++;
		sync_set_idx = (sync_set_idx+1)%CBUF_CT;
	}

	timer.print_fps(frame_ct);

	vkQueueWaitIdle(base.queues.graphics);
	vkQueueWaitIdle(base.queues.present);

	// Cleanup
	for (size_t i = 0; i < CBUF_CT; ++i) {
		vkDestroySemaphore(base.device, image_avail_sems[i], nullptr);
		vkDestroySemaphore(base.device, render_done_sems[i], nullptr);
		vkDestroyFence(base.device, render_done_fences[i], nullptr);
	}

	vkDestroyCommandPool(base.device, cpool, nullptr);

	vkDestroyPipeline(base.device, pipeline, nullptr);
	vkDestroyPipelineLayout(base.device, pipeline_lt, nullptr);
	vkDestroyRenderPass(base.device, rpass, nullptr);

	vs.destroy(base.device);
	fs.destroy(base.device);

	for (auto& i : fbs) vkDestroyFramebuffer(base.device, i, nullptr);

	swapchain.destroy();
}
