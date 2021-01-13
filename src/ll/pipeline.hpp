#ifndef LL_PIPELINE_H
#define LL_PIPELINE_H

#include <vulkan/vulkan.h>

namespace ll::pipeline {
	VkPipelineLayout layout(VkDevice device);

	VkPipeline pipeline(VkDevice device,
			    uint32_t shader_ct, VkPipelineShaderStageCreateInfo* shaders,
			    VkPipelineLayout layout, VkRenderPass rpass);
}

#endif // LL_PIPELINE_H
