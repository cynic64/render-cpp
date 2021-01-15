#ifndef LL_PIPELINE_H
#define LL_PIPELINE_H

#include <vulkan/vulkan.h>

namespace ll::pipeline {
	auto layout(VkDevice device) -> VkPipelineLayout;

	auto pipeline(VkDevice device,
		      uint32_t shader_ct, VkPipelineShaderStageCreateInfo* shaders,
		      VkPipelineLayout layout, VkRenderPass rpass)
		-> VkPipeline;
}

#endif // LL_PIPELINE_H
