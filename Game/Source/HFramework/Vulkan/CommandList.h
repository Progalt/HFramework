#pragma once

#include "VulkanInclude.h"
#include <vector>
#include "Texture.h"
#include "GraphicsPipeline.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Texture.h"

namespace hf
{
	namespace vulkan
	{

		enum class ImageLayout
		{
			Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
			General = VK_IMAGE_LAYOUT_GENERAL,
			ColourAttachmentOptimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			DepthStencilAttachmentOptimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			ShaderReadOnlyOptimal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			PresentSrc = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,

		};

		enum class IndexType
		{
			Uint16, 
			Uint32
		};

		enum class LoadOp
		{
			Clear,
			Load
		};

		enum class StoreOp
		{
			Discard, 
			Store
		};

		union ClearColour
		{
			struct
			{
				float r, g, b, a;
			};

			struct
			{
				float depth;
			};
		};

		struct Attachment
		{
			Texture* texture;
			LoadOp loadOp;
			StoreOp storeOp;
			ClearColour clearColour;
		};

		struct RenderpassInfo
		{
			std::vector<Attachment> colourAttachments;
			Attachment depthAttachment;

		};

		struct BufferImageCopy
		{
			size_t bufferOffset = 0;
			size_t bufferImageHeight = 0;
			size_t bufferRowLength = 0;

			uint32_t mipLevel = 0;
			uint32_t baseArrayLayer = 0;
			uint32_t layerCount = 1;

			struct
			{
				int x = 0, y = 0, z = 0;
			} offset;

			struct
			{
				uint32_t width, height, depth = 1;
			} extent;
		};

		class CommandList
		{
		public:

			void Begin();

			void End();

			void BeginRenderpass(const RenderpassInfo& renderpassInfo);

			void EndRenderpass();

			void SetViewport(int32_t x, int32_t y, int32_t w, int32_t h);

			void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);

			void ResourceBarrier(Texture* texture, ImageLayout newLayout);

			/* -- Drawing Functions -- */

			void Draw(uint32_t vertexCount, uint32_t firstVertex);

			void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t firstVertex = 0);

			/* -- Binding Functions -- */

			void BindPipeline(GraphicsPipeline* pipeline);

			void BindVertexBuffer(Buffer* buffer, uint32_t bindPoint, size_t offset = 0);

			void BindIndexBuffer(Buffer* buffer, IndexType type, size_t offset = 0);

			void BindDescriptorSets(std::vector<DescriptorSet*> sets, uint32_t firstSet);


			/* Copy Functions */

			void CopyBufferToTexture(Buffer* buffer, Texture* texture, const BufferImageCopy& copyInfo);

			void CopyBuffer(Buffer* src, Buffer* dst, size_t size, size_t srcOffset = 0, size_t dstOffset = 0);

		private:

			VkFence m_FinishedExecution;
			

			friend class Device;

			VkCommandBuffer m_Buffer;

			VkDevice m_Device;


			GraphicsPipeline* m_CurrentGraphicsPipeline = nullptr;
			

			bool m_SingleUse = false;
		};
	}
}