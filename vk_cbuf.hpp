#ifndef VK_CBUF_H
#define VK_CBUF_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vk_cbuf {
	void begin(VkCommandBuffer cbuf, VkCommandBufferUsageFlags flags = 0);

	void begin_rpass(VkCommandBuffer cbuf, VkRenderPass rpass,
			 VkFramebuffer fb, uint32_t width, uint32_t height);

	void end_rpass(VkCommandBuffer cbuf);

	void bind_pipeline(VkCommandBuffer cbuf,
			   VkPipeline pipeline, VkPipelineBindPoint point = VK_PIPELINE_BIND_POINT_GRAPHICS);

	void set_viewport(VkCommandBuffer cbuf, std::vector<VkViewport> viewports);

	void set_scissor(VkCommandBuffer cbuf, std::vector<VkRect2D> scissors);

	void draw(VkCommandBuffer cbuf, uint32_t vertex_ct,
		  uint32_t instance_ct = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0);
}

#endif // VK_CBUF_H
