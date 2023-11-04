
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
			if (m_SingleUse)
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			else 
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
					ResourceBarrier(attachment.texture, ImageLayout::ColourAttachmentOptimal);
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

		void CommandList::SetViewport(int32_t x, int32_t y, int32_t w, int32_t h)
		{
			VkViewport viewport{};
			viewport.x = x;
			viewport.y = y;
			viewport.width = w;
			viewport.height = h;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(m_Buffer, 0, 1, &viewport);
		}

		void CommandList::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
		{
			VkRect2D scissor{};
			scissor.offset.x = x;
			scissor.offset.y = y;
			scissor.extent.width = w;
			scissor.extent.height = h;

			vkCmdSetScissor(m_Buffer, 0, 1, &scissor);
		}

		void CommandList::BindPipeline(GraphicsPipeline* pipeline)
		{
			vkCmdBindPipeline(m_Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_Pipeline);

			m_CurrentGraphicsPipeline = pipeline;
		}

		void CommandList::BindVertexBuffer(Buffer* buffer, uint32_t bindPoint, size_t offset)
		{
			VkDeviceSize offsets[] = { offset };
			vkCmdBindVertexBuffers(m_Buffer, bindPoint, 1, &buffer->m_Buffer, offsets);
		}

		void CommandList::BindIndexBuffer(Buffer* buffer, IndexType type, size_t offset)
		{

			VkIndexType idxType;

			// Should Uint32 be our default? or would Uint16 be better
			switch (type)
			{
			case IndexType::Uint16:
				idxType = VK_INDEX_TYPE_UINT16;
				break;
			case IndexType::Uint32:
				idxType = VK_INDEX_TYPE_UINT32;
				break;
			default:
				idxType = VK_INDEX_TYPE_UINT32;
				break;
			}

			vkCmdBindIndexBuffer(m_Buffer, buffer->m_Buffer, offset, idxType);
		}

		void CommandList::BindDescriptorSets(std::vector<DescriptorSet*> sets, uint32_t firstSet)
		{
			if (!m_CurrentGraphicsPipeline)
			{
				Log::Fatal("No Pipeline Bound to bind descriptor set to");
			}

			std::vector<VkDescriptorSet> s(sets.size());

			for (uint32_t i = 0; i < sets.size(); i++)
				s[i] = sets[i]->m_Set;
			
			vkCmdBindDescriptorSets(m_Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurrentGraphicsPipeline->m_Layout, firstSet, s.size(), s.data(), 0, nullptr);
		}

		void CommandList::Draw(uint32_t vertexCount, uint32_t firstVertex)
		{
			vkCmdDraw(m_Buffer, vertexCount, 1, firstVertex, 0);
		}

		void CommandList::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t firstVertex)
		{
			vkCmdDrawIndexed(m_Buffer, indexCount, 1, firstIndex, firstVertex, 0);
		}

		void CommandList::ResourceBarrier(Texture* texture, ImageLayout newLayout)
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

			texture->m_Layout = (VkImageLayout)newLayout;
		}

		void CommandList::CopyBufferToTexture(Buffer* buffer, Texture* texture, const BufferImageCopy& copyInfo)
		{

			VkBufferImageCopy copy{};
			copy.bufferOffset = copyInfo.bufferOffset;
			copy.bufferRowLength = copyInfo.bufferRowLength;
			copy.bufferImageHeight = copyInfo.bufferImageHeight;
			
			copy.imageSubresource.aspectMask = GetAspectMask(texture);
			copy.imageSubresource.mipLevel = copyInfo.mipLevel;
			copy.imageSubresource.baseArrayLayer = copyInfo.baseArrayLayer;
			copy.imageSubresource.layerCount = copyInfo.layerCount;

			copy.imageExtent = { copyInfo.extent.width, copyInfo.extent.height, copyInfo.extent.depth };
			copy.imageOffset = { copyInfo.offset.x, copyInfo.offset.y, copyInfo.offset.z };


			vkCmdCopyBufferToImage(m_Buffer, buffer->m_Buffer, texture->m_Image, texture->m_Layout, 1, &copy);

		}

		void CommandList::CopyBuffer(Buffer* src, Buffer* dst, size_t size, size_t srcOffset, size_t dstOffset)
		{
			VkBufferCopy copy{};
			copy.size = size;
			copy.dstOffset = dstOffset;
			copy.srcOffset = srcOffset;

			vkCmdCopyBuffer(m_Buffer, src->m_Buffer, dst->m_Buffer, 1, &copy);
		}
	}
}