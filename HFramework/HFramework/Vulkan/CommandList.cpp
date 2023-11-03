
#include "CommandList.h"
#include "../Core/Log.h"
#include "TextureUtil.h"

namespace hf
{
	namespace vulkan
	{
		void CommandList::Begin()
		{
			if (m_FinishedExecution)
			{
				vkWaitForFences(m_Device, 1, &m_FinishedExecution, VK_TRUE, UINT64_MAX);
			}

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(m_Buffer, &beginInfo) != VK_SUCCESS)
			{
				Log::Fatal("Failed to begin command list");
			}

		}

		void CommandList::End()
		{
			if (vkEndCommandBuffer(m_Buffer) != VK_SUCCESS)
			{
				Log::Fatal("Failed to end command list recording");
			}
		}

		void CommandList::BeginRenderpass(const RenderpassInfo& renderpassInfo)
		{
			std::vector<VkRenderingAttachmentInfo> colourAttachmentInfos(renderpassInfo.colourAttachments.size());

			uint32_t idx = 0;
			for (auto& attachment : renderpassInfo.colourAttachments)
			{
				VkRenderingAttachmentInfo info{};
				info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				info.imageView = attachment.texture->m_ImageView;
				
				switch (attachment.loadOp)
				{
				case LoadOp::Clear:
					info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					break;
				case LoadOp::Load:
					info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
					break;
				}

				switch (attachment.storeOp)
				{
				case StoreOp::Store:
					info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					break;
				case StoreOp::Discard:
					info.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
					break;
				}

				info.clearValue.color.float32[0] = attachment.clearColour.r;
				info.clearValue.color.float32[1] = attachment.clearColour.g;
				info.clearValue.color.float32[2] = attachment.clearColour.b;
				info.clearValue.color.float32[3] = attachment.clearColour.a;

				colourAttachmentInfos[idx] = info;

				// We need to transition the layout if it doesn't match 
				if (attachment.texture->m_Format != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				{
					ResourceBarrier(attachment.texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				}
			}

			VkRenderingInfo renderInfo{};
			renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			renderInfo.layerCount = 1;

			// TODO: Make this better
			VkRect2D renderArea;
			renderArea.offset = { 0, 0 };
			renderArea.extent = { renderpassInfo.colourAttachments[0].texture->m_Width, renderpassInfo.colourAttachments[0].texture->m_Height };
			renderInfo.renderArea = renderArea;

			renderInfo.colorAttachmentCount = colourAttachmentInfos.size();
			renderInfo.pColorAttachments = colourAttachmentInfos.data();

			vkCmdBeginRendering(m_Buffer, &renderInfo);

		}

		void CommandList::EndRenderpass()
		{
			vkCmdEndRendering(m_Buffer);
		}

		void CommandList::ResourceBarrier(Texture* texture, VkImageLayout newLayout)
		{
			VkImageMemoryBarrier imgBarrier = {};
			imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imgBarrier.pNext = nullptr;
			imgBarrier.oldLayout = static_cast<VkImageLayout>(texture->m_Layout);
			imgBarrier.newLayout = static_cast<VkImageLayout>(newLayout);
			imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imgBarrier.image = texture->m_Image;
			imgBarrier.subresourceRange.aspectMask = GetAspectMask(texture);
			imgBarrier.subresourceRange.baseMipLevel = 0;
			imgBarrier.subresourceRange.levelCount = 1;
			imgBarrier.subresourceRange.baseArrayLayer = 0;
			imgBarrier.subresourceRange.layerCount = 1;
			imgBarrier.srcAccessMask = GetAccessMaskFromLayout(imgBarrier.oldLayout, false);
			imgBarrier.dstAccessMask = GetAccessMaskFromLayout(imgBarrier.newLayout, true);

			VkPipelineStageFlags sourceStageMask = 0;
			{
				if (imgBarrier.oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				{
					sourceStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				}
				else if (imgBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				{
					sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				}
				else
				{
					sourceStageMask = AccessFlagsToPipelineStage(imgBarrier.srcAccessMask);
				}
			}

			VkPipelineStageFlags destStageMask = 0;
			{
				if (imgBarrier.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				{
					destStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				}
				else
				{
					destStageMask = AccessFlagsToPipelineStage(imgBarrier.dstAccessMask);
				}
			}

			vkCmdPipelineBarrier
			(
				m_Buffer,
				sourceStageMask,
				destStageMask,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imgBarrier
			);

			texture->m_Layout = newLayout;
		}
	}
}