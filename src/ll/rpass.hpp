#ifndef LL_RPASS_H_
#define LL_RPASS_H_

#include <vulkan/vulkan.h>
#include <vector>

namespace ll::rpass {
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

	const VkSubpassDependency DEPENDENCY_DEFAULTS {
		VK_SUBPASS_EXTERNAL, // srcSubpass 
		0, // dstSubpass 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask 
		0, // srcAccessMask 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask 
		0 // dependencyFlags
	};

	VkAttachmentDescription attachment(VkFormat format,
					   AttachmentSettings settings = ATTACHMENT_DEFAULTS);

	VkAttachmentReference attachment_ref(uint32_t idx, VkImageLayout layout);

	VkSubpassDescription subpass(uint32_t color_ref_ct, VkAttachmentReference* color_refs,
				     SubpassSettings settings = SUBPASS_DEFAULTS);

	VkSubpassDependency dependency(VkSubpassDependency settings = DEPENDENCY_DEFAULTS);

	VkRenderPass rpass(VkDevice device,
			   uint32_t attachment_ct, VkAttachmentDescription* attachments,
			   uint32_t subpass_ct, VkSubpassDescription* subpasses,
			   uint32_t dependecy_ct, VkSubpassDependency* dependencies);
}

#endif // LL_RPASS_H_
