#include "vk_pipeline.hpp"

#include <stdexcept>

namespace vk_pipeline {
	VkPipelineLayout layout(VkDevice device) {
		VkPipelineLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkPipelineLayout layout;
		if (vkCreatePipelineLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS)
			throw std::runtime_error("Couldn't create pipeline layout!");

		return layout;
	}

	VkPipeline pipeline(VkDevice device,
			    uint32_t shader_ct, VkPipelineShaderStageCreateInfo* shaders,
			    VkPipelineLayout layout, VkRenderPass rpass) {
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
		pipeline_info.stageCount = shader_ct;
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

		return pipeline;
	}
}
