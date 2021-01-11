#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_queue.hpp"
#include "vk_swapchain.hpp"
#include "vk_image.hpp"
#include "vk_shader.hpp"
#include "vk_rpass.hpp"
#include "vk_pipeline.hpp"
#include "vk_cbuf.hpp"
#include "timer.hpp"
#include "glfw_window.hpp"

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
	auto glfw_window = glfw_window::GWindow(INIT_WIDTH, INIT_HEIGHT);
	glfwSetFramebufferSizeCallback(glfw_window.window, resize_callback);

	VkInstance instance;
	// Will only actually be set if validation is enabled
	VkDebugUtilsMessengerEXT debug_msgr{};
	vk_instance::create(glfw_window.req_instance_exts, {}, VALIDATION_ENABLED, &instance, &debug_msgr);

	// Create surface
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, glfw_window.window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Could not create surface!");

	// Create physical device
	VkPhysicalDevice phys_dev;
	auto phys_dev_name = vk_phys_dev::create(instance, vk_phys_dev::default_scorer, &phys_dev);
	std::cout << "Using device: " << phys_dev_name << std::endl;

	// Find queue families
	vk_queue::QueueFamilies queue_fams(phys_dev, surface);
	if (!queue_fams.graphics.has_value() || !queue_fams.present.has_value())
		throw std::runtime_error("Unable to find necessary queue families!");

	// Create logical device
	// Necessary because the device has to support all our different queue
	// families
	auto dev_queue_infos = vk_device::default_queue_infos(queue_fams.unique.begin(), queue_fams.unique.end());

	VkDevice device;
	std::vector<const char *> dev_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	vk_device::create(phys_dev, dev_queue_infos, {}, dev_extensions, &device);

	// Retrieve queues
	VkQueue queue_graphics, queue_present;
	vkGetDeviceQueue(device, queue_fams.graphics.value(), 0, &queue_graphics);
	if (queue_fams.present.value() != queue_fams.graphics.value())
		vkGetDeviceQueue(device, queue_fams.present.value(), 0, &queue_present);
	else queue_present = queue_graphics;

	// Load shaders
	auto vs = vk_shader::Shader(device, VK_SHADER_STAGE_VERTEX_BIT, "../shader.vert.spv");
	auto fs = vk_shader::Shader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "../shader.frag.spv");

	VkPipelineShaderStageCreateInfo shaders[] = {vs.info, fs.info};

	auto pipeline_lt = vk_pipeline::layout(device);

	// Allocate command buffer
	VkCommandPoolCreateInfo cpool_info{};
	cpool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpool_info.queueFamilyIndex = queue_fams.graphics.value();
	cpool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool cpool;
	if (vkCreateCommandPool(device, &cpool_info, nullptr, &cpool) != VK_SUCCESS)
		throw std::runtime_error("Could not create command pool!");

	VkCommandBufferAllocateInfo cbuf_info{};
	cbuf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbuf_info.commandPool = cpool;
	cbuf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbuf_info.commandBufferCount = CBUF_CT;

	std::vector<VkCommandBuffer> cbufs(CBUF_CT);
	if (vkAllocateCommandBuffers(device, &cbuf_info, cbufs.data()) != VK_SUCCESS)
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

	VkSemaphoreCreateInfo sem_info{};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < CBUF_CT; ++i) {
		vkCreateSemaphore(device, &sem_info, nullptr, &image_avail_sems[i]);
		vkCreateSemaphore(device, &sem_info, nullptr, &render_done_sems[i]);
		vkCreateFence(device, &fence_info, nullptr, &render_done_fences[i]);
	}

	// Everything from here on depends on the swapchain, so we make them
	// null for now
	vk_swapchain::Swapchain swapchain;
	VkRenderPass rpass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> fbs;
	std::vector<VkFence> image_fences;

	auto sync_set_idx = 0;

	// Main loop
	timer::Timer timer;
	size_t frame_ct = 0;
	must_recreate = true;

	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();

		while (must_recreate) {
			vkDeviceWaitIdle(device);
			// Clean up old stuff
			if (swapchain.swapchain != VK_NULL_HANDLE) swapchain.destroy();

			// Create swapchain
			auto [wwidth, wheight] = glfw_window.get_dims();
			swapchain = vk_swapchain::Swapchain(phys_dev, device, surface,
							    VK_NULL_HANDLE,
							    queue_fams.unique.size(), queue_fams.unique.data(),
							    wwidth, wheight);
			auto [newwidth, newwheight] = glfw_window.get_dims();
			if (newwidth != wwidth || newwheight != wheight) continue;

			if (rpass != VK_NULL_HANDLE) vkDestroyRenderPass(device, rpass, nullptr);
			if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, pipeline, nullptr);
			if (!fbs.empty()) for (auto& i : fbs) vkDestroyFramebuffer(device, i, nullptr);

			// Create render pass
			auto color_attachment = vk_rpass::attachment(swapchain.format);
			auto color_ref = vk_rpass::attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			auto subpass = vk_rpass::subpass(1, &color_ref);
			auto subpass_dep = vk_rpass::dependency();
			rpass = vk_rpass::rpass(device, 1, &color_attachment, 1, &subpass, 1, &subpass_dep);

			// Create pipeline
			pipeline = vk_pipeline::pipeline(device, 2, shaders, pipeline_lt, rpass);

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
				
				if (vkCreateFramebuffer(device, &info, nullptr, &fbs[i]) != VK_SUCCESS)
					throw std::runtime_error("Could not create framebuffer!");
			}

			// Clear image fences
			image_fences.clear();
			image_fences.resize(swapchain.images.size(), VK_NULL_HANDLE);

			// Recreate fences and semaphores
			for (size_t i = 0; i < CBUF_CT; ++i) {
				vkDestroySemaphore(device, image_avail_sems[i], nullptr);
				vkDestroySemaphore(device, render_done_sems[i], nullptr);
				vkDestroyFence(device, render_done_fences[i], nullptr);

				vkCreateSemaphore(device, &sem_info, nullptr, &image_avail_sems[i]);
				vkCreateSemaphore(device, &sem_info, nullptr, &render_done_sems[i]);
				vkCreateFence(device, &fence_info, nullptr, &render_done_fences[i]);
			}


			// Update dynamic state
			viewport.width = swapchain.width;
			viewport.height = swapchain.width;
			scissor.extent.width = swapchain.width;
			scissor.extent.height = swapchain.width;

			must_recreate = false;
		}

		// Wait for the sync set we'll use to become available
		if (vkWaitForFences(device, 1, &render_done_fences[sync_set_idx], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			throw std::runtime_error("Could not wait for sync set's render-done fence!");

		auto cbuf = cbufs[sync_set_idx];
		vkResetCommandBuffer(cbuf, 0);

		uint32_t image_idx = 0;
		if (vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX,
				      image_avail_sems[sync_set_idx], VK_NULL_HANDLE, &image_idx)
		    != VK_SUCCESS) {
			must_recreate = true;
			continue;
		}

		// Wait for whoever's drawing to our image to finish
		if (image_fences[image_idx] != VK_NULL_HANDLE)
			if (vkWaitForFences(device, 1, &image_fences[image_idx], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
				throw std::runtime_error("Could not wait for image's fence!");

		if (vkResetFences(device, 1, &render_done_fences[sync_set_idx]) != VK_SUCCESS)
			throw std::runtime_error("Could not reset render-done fence!");

		// We're now rendering to this image, so mark it with our fence
		image_fences[image_idx] = render_done_fences[sync_set_idx];

		vk_cbuf::begin(cbuf);
		vk_cbuf::begin_rpass(cbuf, rpass, fbs[image_idx], swapchain.width, swapchain.height);
		vk_cbuf::bind_pipeline(cbuf, pipeline);
		vk_cbuf::set_viewport(cbuf, {viewport});
		vk_cbuf::set_scissor(cbuf, {scissor});
		vk_cbuf::draw(cbuf, 3);
		vk_cbuf::end_rpass(cbuf);

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

		if (must_recreate) continue;

		if (vkQueueSubmit(queue_graphics, 1, &submit_info, render_done_fences[sync_set_idx]) != VK_SUCCESS)
			throw std::runtime_error("Could not submit!");

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_done_sems[sync_set_idx];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain.swapchain;
		present_info.pImageIndices = &image_idx;

		if (must_recreate) continue;

		auto res = vkQueuePresentKHR(queue_present, &present_info);
		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			must_recreate = true;
		} else if (res != VK_SUCCESS)
			throw std::runtime_error("Presenting failed with something other than out-of-date!");

		frame_ct++;
		sync_set_idx = (sync_set_idx+1)%CBUF_CT;
	}

	timer.print_fps(frame_ct);

	vkQueueWaitIdle(queue_graphics);
	vkQueueWaitIdle(queue_present);

	// Cleanup
	for (size_t i = 0; i < CBUF_CT; ++i) {
		vkDestroySemaphore(device, image_avail_sems[i], nullptr);
		vkDestroySemaphore(device, render_done_sems[i], nullptr);
		vkDestroyFence(device, render_done_fences[i], nullptr);
	}

	vkDestroyCommandPool(device, cpool, nullptr);

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline_lt, nullptr);
	vkDestroyRenderPass(device, rpass, nullptr);

	vs.destroy(device);
	fs.destroy(device);

	for (auto& i : fbs) vkDestroyFramebuffer(device, i, nullptr);

	swapchain.destroy();

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_msgr) vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
