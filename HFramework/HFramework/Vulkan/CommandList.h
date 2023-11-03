#pragma once

#include "VulkanInclude.h"
#include <vector>
#include "Texture.h"

namespace hf
{
	namespace vulkan
	{
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

		class CommandList
		{
		public:

			void Begin();

			void End();

			void BeginRenderpass(const RenderpassInfo& renderpassInfo);

			void ResourceBarrier(Texture* texture, VkImageLayout newLayout);

			void EndRenderpass();

		private:

			VkFence m_FinishedExecution;
			

			friend class Device;

			VkCommandBuffer m_Buffer;

			VkDevice m_Device;
		};
	}
}