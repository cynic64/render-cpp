#ifndef LL_SHADER_H
#define LL_SHADER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace ll::shader {
	using Shader = VkPipelineShaderStageCreateInfo;

	auto create(VkDevice device, VkShaderStageFlagBits stage, const std::vector<char>& bytes) -> Shader;

	auto create(VkDevice device, VkShaderStageFlagBits stage, const char* filename) -> Shader;

	void destroy(VkDevice device, Shader shader);
}

#endif // LL_SHADER_H
