#pragma once
#include "VulkanInclude.h"

namespace hf
{
	namespace vulkan
	{

		enum class BufferUsage
		{
			Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			ShaderStorage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			IndirectArguments = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};

		inline BufferUsage operator|(BufferUsage lh, BufferUsage rh)
		{
			return static_cast<BufferUsage>(
					static_cast<int>(lh) | static_cast<int>(rh)
				);
		}

		enum class BufferVisibility
		{
			HostVisible,	/* Buffer is visible to host and device */
			Device			/* Buffer is only visible to device*/
		};

		struct BufferDesc
		{
			BufferUsage usage;
			BufferVisibility visibility;
			size_t bufferSize;
		};

		class Buffer
		{
		public:

			void Dispose();

			void* Map();

			void Unmap();

		private:

			friend class Device;
			friend class CommandList;
			friend class DescriptorSet;

			bool m_Mapped = false;
			void* m_MappedBuffer = nullptr;

			VkBuffer m_Buffer;
			VmaAllocation m_Allocation;

			// Must be set by the device
			VmaAllocator m_AssociatedAllocator;
			VkDevice m_AssociatedDevice;

			void Create(const BufferDesc& desc);
		};

	}
}
