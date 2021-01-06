#ifndef VK_SHADER_H
#define VK_SHADER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vk_shader {
	class Shader {
	public:
		Shader(VkDevice device, VkShaderStageFlagBits stage, std::vector<char> bytes);
		Shader(VkDevice device, VkShaderStageFlagBits stage, const char * filename);

		void destroy(VkDevice device);

		VkPipelineShaderStageCreateInfo info;
	};

	std::vector<char> read_bytes(const char* filename);
}

#endif // VK_SHADER_H
