
#include "Buffer.h"
#include "../Core/Log.h"

namespace hf
{
	namespace vulkan
	{
		void Buffer::Dispose()
		{
			if (m_Mapped)
			{
				vmaUnmapMemory(m_AssociatedAllocator, m_Allocation);
			}

			vmaDestroyBuffer(m_AssociatedAllocator, m_Buffer, m_Allocation);
		}

		void* Buffer::Map()
		{
			if (m_Mapped)
			{
				return m_MappedBuffer;
			}

			VkResult res = vmaMapMemory(m_AssociatedAllocator, m_Allocation, &m_MappedBuffer);

			if (res != VK_SUCCESS)
			{
				Log::Error("Failed to map buffer");

				return nullptr;
			}

			m_Mapped = true;

			return m_MappedBuffer;
		}

		void Buffer::Unmap()
		{
			if (!m_Mapped)
			{
				Log::Warn("Attempting to unmap buffer which wasn't mapped in the first place");
				return;
			}

			vmaUnmapMemory(m_AssociatedAllocator, m_Allocation);
			
			m_Mapped = false;
		}


		void Buffer::Create(const BufferDesc& desc)
		{
			VkBufferUsageFlagBits  usage = (VkBufferUsageFlagBits)desc.usage;

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = desc.bufferSize;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

			if (desc.visibility == BufferVisibility::HostVisible)
			{
				allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			}

			if (vmaCreateBuffer(m_AssociatedAllocator, &bufferInfo, &allocInfo, &m_Buffer, &m_Allocation, nullptr) != VK_SUCCESS)
			{
				Log::Error("Failed to Create Buffer");
				return;
			}

			Log::Info("Successfully Created Buffer");

		}
	}
}