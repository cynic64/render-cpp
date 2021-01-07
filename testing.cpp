#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_phys_dev.hpp"
#include "vk_queue.hpp"
#include "vk_swapchain.hpp"
#include "vk_image.hpp"
#include "vk_shader.hpp"
#include "vk_rpass.hpp"

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
	VkDevice device;

	// Necessary because the device has to support all our different queue
	// families
	auto dev_queue_infos = vk_device::default_queue_infos(queue_fams.unique.begin(), queue_fams.unique.end());

	std::vector<const char *> dev_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	vk_device::create(phys_dev, dev_queue_infos, {}, dev_extensions, &device);

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
	auto rpass = vk_rpass::rpass(device, 1, &color_attachment, 1, &subpass);

	// Create pipeline layout
	VkPipelineLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("Couldn't create pipeline layout!");

	// Create graphics pipeline
	VkPipelineVertexInputStateCreateInfo vertex_input{};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport{};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// Will be handled by dynamic state
	viewport.viewportCount = 1;
	viewport.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState attachment_blend{};
	attachment_blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachment_blend.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo blending{};
	blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending.logicOpEnable = VK_FALSE;
	blending.attachmentCount = 1;
	blending.pAttachments = &attachment_blend;

	VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dyn_state{};
	dyn_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn_state.dynamicStateCount = std::extent<decltype(dyn_states)>::value;
	dyn_state.pDynamicStates = dyn_states;

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shaders;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &blending;
	pipeline_info.pDynamicState = &dyn_state;
	pipeline_info.layout = layout;
	pipeline_info.renderPass = rpass;
	pipeline_info.subpass = 0;

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Could not create pipeline!");

	// Choose swapchain settings
	while (!glfwWindowShouldClose(glfw_window.window)) {
		glfwPollEvents();
	}

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, layout, nullptr);
	vkDestroyRenderPass(device, rpass, nullptr);

	vs.destroy(device);
	fs.destroy(device);

	swapchain.destroy();

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (debug_msgr) vk_instance::destroy_debug_msgr(instance, debug_msgr);
	vkDestroyInstance(instance, nullptr);
}
