#include "shader.hpp"

#include <fstream>
#include <stdexcept>

namespace ll::shader {
	auto read_bytes(const char* filename) -> std::vector<char> {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) throw std::runtime_error("Could not open file!");

		size_t byte_ct = file.tellg();
		std::vector<char> buffer(byte_ct);

		file.seekg(0);
		file.read(buffer.data(), byte_ct);

		file.close();

		return buffer;
	}

	auto create_module(VkDevice device, const std::vector<char>& bytes) -> VkShaderModule {
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = bytes.size();
		info.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

		VkShaderModule shader;
		if (vkCreateShaderModule(device, &info, nullptr, &shader) != VK_SUCCESS)
			throw std::runtime_error("Could not create shader module!");

		return shader;
	}

	auto create(VkDevice device, VkShaderStageFlagBits stage, const std::vector<char>& bytes) -> Shader {
		Shader shader{};
		auto module = create_module(device, bytes); 

		shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader.stage = stage;
		shader.module = module;
		shader.pName = "main";

		return shader;
	}

	auto create(VkDevice device, VkShaderStageFlagBits stage, const char* filename) -> Shader {
		return create(device, stage, read_bytes(filename));
	}

	void destroy(VkDevice device, Shader shader) {
		vkDestroyShaderModule(device, shader.module, nullptr);
	}
}
