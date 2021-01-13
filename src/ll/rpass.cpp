#include "rpass.hpp"

#include <stdexcept>

namespace ll::rpass {
	VkAttachmentDescription attachment(VkFormat format,
					   AttachmentSettings settings) {
		VkAttachmentDescription info{};
		info.format = format;
		info.samples = settings.samples;
		info.loadOp = settings.load;
		info.storeOp = settings.store;
		info.stencilLoadOp = settings.stencil_load;
		info.stencilStoreOp = settings.stencil_store;
		info.initialLayout = settings.initial_layout;
		info.finalLayout = settings.final_layout;

		return info;
	}

	VkAttachmentReference attachment_ref(uint32_t idx, VkImageLayout layout) {
		VkAttachmentReference info{};
		info.attachment = idx;
		info.layout = layout;

		return info;
	}

	VkSubpassDescription subpass(uint32_t color_ref_ct, VkAttachmentReference* color_refs,
				     SubpassSettings settings) {
		VkSubpassDescription info{};
		info.pipelineBindPoint = settings.bind_point;
		info.colorAttachmentCount = color_ref_ct;
		info.pColorAttachments = color_refs;

		return info;
	}

	VkSubpassDependency dependency(VkSubpassDependency settings) {
		// So useful...
		return settings;
	}

	VkRenderPass rpass(VkDevice device,
			   uint32_t attachment_ct, VkAttachmentDescription* attachments,
			   uint32_t subpass_ct, VkSubpassDescription* subpasses,
			   uint32_t dependecy_ct, VkSubpassDependency* dependencies) {
		VkRenderPassCreateInfo rpass_info{};
		rpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpass_info.attachmentCount = attachment_ct;
		rpass_info.pAttachments = attachments;
		rpass_info.subpassCount = subpass_ct;
		rpass_info.pSubpasses = subpasses;
		rpass_info.dependencyCount = dependecy_ct;
		rpass_info.pDependencies = dependencies;

		VkRenderPass rpass;
		if (vkCreateRenderPass(device, &rpass_info, nullptr, &rpass) != VK_SUCCESS)
			throw std::runtime_error("Could not create render pass!");

		return rpass;
	}
}
