#include "shader.hpp"

#include <fstream>
#include <stdexcept>

namespace ll::shader {
	std::vector<char> read_bytes(const char* filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) throw std::runtime_error("Could not open file!");

		size_t byte_ct = file.tellg();
		std::vector<char> buffer(byte_ct);

		file.seekg(0);
		file.read(buffer.data(), byte_ct);

		file.close();

		return buffer;
	}

	VkShaderModule create_module(VkDevice device, const std::vector<char>& bytes) {
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = bytes.size();
		info.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

		VkShaderModule shader;
		if (vkCreateShaderModule(device, &info, nullptr, &shader) != VK_SUCCESS)
			throw std::runtime_error("Could not create shader module!");

		return shader;
	}

	Shader::Shader(VkDevice device, VkShaderStageFlagBits stage, std::vector<char> bytes) : info({}) {
		auto module = create_module(device, bytes); 

		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage = stage;
		info.module = module;
		info.pName = "main";
	}

	Shader::Shader(VkDevice device, VkShaderStageFlagBits stage, const char* filename)
		: Shader(device, stage, read_bytes(filename)) {}

	void Shader::destroy(VkDevice device) {
		vkDestroyShaderModule(device, info.module, nullptr);
	}
}
