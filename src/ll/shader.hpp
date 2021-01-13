#ifndef LL_SHADER_H
#define LL_SHADER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace ll::shader {
	class Shader {
	public:
		Shader(VkDevice device, VkShaderStageFlagBits stage, std::vector<char> bytes);
		Shader(VkDevice device, VkShaderStageFlagBits stage, const char* filename);

		void destroy(VkDevice device);

		VkPipelineShaderStageCreateInfo info;
	};

	std::vector<char> read_bytes(const char* filename);
}

#endif // LL_SHADER_H
