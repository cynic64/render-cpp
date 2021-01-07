#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include <vulkan/vulkan.h>

namespace vk_pipeline {
	VkPipelineLayout layout(VkDevice device);

	VkPipeline pipeline(VkDevice device,
			    uint32_t shader_ct, VkPipelineShaderStageCreateInfo* shaders,
			    VkPipelineLayout layout, VkRenderPass rpass);
}

#endif // VK_PIPELINE_H
