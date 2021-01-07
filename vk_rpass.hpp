#ifndef VK_RPASS_H_
#define VK_RPASS_H_

#include <vulkan/vulkan.h>
#include <vector>

namespace vk_rpass {
	struct AttachmentSettings {
		VkSampleCountFlagBits samples;
		VkAttachmentLoadOp load;
		VkAttachmentStoreOp store;
		VkAttachmentLoadOp stencil_load;
		VkAttachmentStoreOp stencil_store;
		VkImageLayout initial_layout;
		VkImageLayout final_layout;
	};

	const AttachmentSettings ATTACHMENT_DEFAULTS {
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	struct SubpassSettings {
		VkPipelineBindPoint bind_point;
	};

	const SubpassSettings SUBPASS_DEFAULTS {
		VK_PIPELINE_BIND_POINT_GRAPHICS
	};

	VkAttachmentDescription attachment(VkFormat format,
					   AttachmentSettings settings = ATTACHMENT_DEFAULTS);

	VkAttachmentReference attachment_ref(uint32_t idx, VkImageLayout layout);

	VkSubpassDescription subpass(uint32_t color_ref_ct, VkAttachmentReference* color_refs,
				     SubpassSettings settings = SUBPASS_DEFAULTS);

	VkRenderPass rpass(VkDevice device,
			   uint32_t attachment_ct, VkAttachmentDescription* attachments,
			   uint32_t subpass_ct, VkSubpassDescription* subpasses);
}

#endif // VK_RPASS_H_
