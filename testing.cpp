#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_queue.hpp"
#include "vk_swapchain.hpp"
#include "vk_image.hpp"
#include "vk_shader.hpp"
#include "vk_rpass.hpp"
#include "vk_pipeline.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <type_traits>

const uint32_t INIT_WIDTH = 800, INIT_HEIGHT = 600;

#ifdef NDEBUG
const bool VALIDATION_ENABLED = false;
#else
const bool VALIDATION_ENABLED = true;
#endif

struct GWindow {
	GLFWwindow* window;

	// Pointers are valid until GLFW terminates (so until the GWindow is
	// destroyed)
	std::vector<const char*> req_instance_exts;

	GWindow(uint32_t width, uint32_t height) {
		glfwInit();
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

		const char** raw_extensions;
		uint32_t extension_ct;
		raw_extensions = glfwGetRequiredInstanceExtensions(&extension_ct);
		req_instance_exts = {raw_extensions, raw_extensions + extension_ct};
	}

	std::pair<int, int> get_dims() {
		std::pair<int, int> dims;
		glfwGetFramebufferSize(window, &dims.first, &dims.second);

		return dims;
	}

	~GWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	auto glfw_window = GWindow(INIT_WIDTH, INIT_HEIGHT);

	VkInstance instance;
	// Will only actually be set if validation is enabled
	VkDebugUtilsMessengerEXT debug_msgr{};
	vk_instance::create(glfw_window.req_instance_exts, {}, VALIDATION_ENABLED, &instance, &debug_msgr);

	// Create surface
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, glfw_window.window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("Could not create surface!");

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

	// Create swapchain
	auto [wwidth, wheight] = glfw_window.get_dims();
	auto swapchain = vk_swapchain::Swapchain(phys_dev, device, surface,
						 VK_NULL_HANDLE, queue_fams.unique.size(), queue_fams.unique.data(),
						 wwidth, wheight);

	auto vs = vk_shader::Shader(device, VK_SHADER_STAGE_VERTEX_BIT, "../shader.vert.spv");
	auto fs = vk_shader::Shader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "../shader.frag.spv");

	VkPipelineShaderStageCreateInfo shaders[] = {vs.info, fs.info};

	// Create render pass
	auto color_attachment = vk_rpass::attachment(swapchain.format);
	auto color_ref = vk_rpass::attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	auto subpass = vk_rpass::subpass(1, &color_ref);
	auto subpass_dep = vk_rpass::dependency();
	auto rpass = vk_rpass::rpass(device, 1, &color_attachment, 1, &subpass, 1, &subpass_dep);

	// Create pipeline
	auto pipeline_lt = vk_pipeline::layout(device);
	auto pipeline = vk_pipeline::pipeline(device, 2, shaders, pipeline_lt, rpass);

	// Create framebuffers
	std::vector<VkFramebuffer> fbs(swapchain.images.size());
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
	cbuf_info.commandBufferCount = 1;

	VkCommandBuffer cbuf;
	if (vkAllocateCommandBuffers(device, &cbuf_info, &cbuf) != VK_SUCCESS)
		throw std::runtime_error("Could not allocate command buffer!");

	// Dynamic state
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = swapchain.width;
	viewport.height = swapchain.width;
	viewport.minDepth = 0.0f;
	viewport.minDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent.width = swapchain.width;
	scissor.extent.height = swapchain.width;

	// Synchronization
	VkSemaphore image_avail_sem, render_done_sem;
	VkSemaphoreCreateInfo sem_info{};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(device, &sem_info, nullptr, &image_avail_sem) != VK_SUCCESS
	    || vkCreateSemaphore(device, &sem_info, nullptr, &render_done_sem) != VK_SUCCESS)
		throw std::runtime_error("Could not create semaphores!");

	// Main loop
	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();

		uint32_t image_idx = 0;
		vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX,
				      image_avail_sem, VK_NULL_HANDLE, &image_idx);

		VkCommandBufferBeginInfo cbuf_begin{};
		cbuf_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbuf_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(cbuf, &cbuf_begin) != VK_SUCCESS)
			throw std::runtime_error("Could not begin command buffer!");

		VkRenderPassBeginInfo cbuf_rpass_info{};
		cbuf_rpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		cbuf_rpass_info.renderPass = rpass;
		cbuf_rpass_info.framebuffer = fbs[image_idx];
		cbuf_rpass_info.renderArea.offset = {0, 0};
		cbuf_rpass_info.renderArea.extent.width = swapchain.width;
		cbuf_rpass_info.renderArea.extent.height = swapchain.height;
		VkClearValue clear_val = {0.0f, 0.0f, 0.0f, 0.0f};
		cbuf_rpass_info.clearValueCount = 1;
		cbuf_rpass_info.pClearValues = &clear_val;
		vkCmdBeginRenderPass(cbuf, &cbuf_rpass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(cbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdSetViewport(cbuf, 0, 1, &viewport);
		vkCmdSetScissor(cbuf, 0, 1, &scissor);

		vkCmdDraw(cbuf, 3, 1, 0, 0);

		vkCmdEndRenderPass(cbuf);
		if (vkEndCommandBuffer(cbuf) != VK_SUCCESS)
			throw std::runtime_error("Could not end command buffer!");

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_avail_sem;
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cbuf;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_done_sem;

		if (vkQueueSubmit(queue_graphics, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
			throw std::runtime_error("Could not submit!");

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_done_sem;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain.swapchain;
		present_info.pImageIndices = &image_idx;
		vkQueuePresentKHR(queue_present, &present_info);

		vkQueueWaitIdle(queue_graphics);
		vkQueueWaitIdle(queue_present);

		vkResetCommandBuffer(cbuf, 0);
	}

	// Cleanup
	vkDestroySemaphore(device, image_avail_sem, nullptr);
	vkDestroySemaphore(device, render_done_sem, nullptr);

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
