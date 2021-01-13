#include "cbuf.hpp"

#include <stdexcept>

namespace ll::cbuf {
	void begin(VkCommandBuffer cbuf, VkCommandBufferUsageFlags flags) {
		VkCommandBufferBeginInfo cbuf_begin{};
		cbuf_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbuf_begin.flags = flags;

		if (vkBeginCommandBuffer(cbuf, &cbuf_begin) != VK_SUCCESS)
			throw std::runtime_error("Could not begin command buffer!");
	}

	void begin_rpass(VkCommandBuffer cbuf, VkRenderPass rpass,
			 VkFramebuffer fb, uint32_t width, uint32_t height) {
		VkRenderPassBeginInfo cbuf_rpass_info{};
		cbuf_rpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		cbuf_rpass_info.renderPass = rpass;
		cbuf_rpass_info.framebuffer = fb;
		cbuf_rpass_info.renderArea.offset = {0, 0};
		cbuf_rpass_info.renderArea.extent.width = width;
		cbuf_rpass_info.renderArea.extent.height = height;
		VkClearValue clear_val = {0.0f, 0.0f, 0.0f, 0.0f};
		cbuf_rpass_info.clearValueCount = 1;
		cbuf_rpass_info.pClearValues = &clear_val;
		vkCmdBeginRenderPass(cbuf, &cbuf_rpass_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void bind_pipeline(VkCommandBuffer cbuf,
			   VkPipeline pipeline, VkPipelineBindPoint point) {
		vkCmdBindPipeline(cbuf, point, pipeline);
	}

	void set_viewport(VkCommandBuffer cbuf, std::vector<VkViewport> viewports) {
		vkCmdSetViewport(cbuf, 0, viewports.size(), viewports.data());
	}

	void set_scissor(VkCommandBuffer cbuf, std::vector<VkRect2D> scissors) {
		vkCmdSetScissor(cbuf, 0, scissors.size(), scissors.data());
	}

	void draw(VkCommandBuffer cbuf, uint32_t vertex_ct,
		  uint32_t instance_ct, uint32_t first_vertex, uint32_t first_instance) {
		vkCmdDraw(cbuf, vertex_ct, instance_ct, first_vertex, first_instance);
	}

	void end_rpass(VkCommandBuffer cbuf) {
		vkCmdEndRenderPass(cbuf);
		if (vkEndCommandBuffer(cbuf) != VK_SUCCESS)
			throw std::runtime_error("Could not end command buffer!");
	}
}
