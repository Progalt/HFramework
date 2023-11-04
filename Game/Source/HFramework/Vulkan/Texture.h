#pragma once
#include "VulkanInclude.h"
#include "../Graphics/Format.h"

namespace hf
{
	namespace vulkan
	{
		enum class TextureType
		{
			Flat2D,
			Array2D, 
			Flat3D,
		};

		struct TextureDesc
		{
			Format format;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t depth = 1;
			uint32_t mipLevels = 1;
			uint32_t arrayLevels = 1;
			TextureType type;
			bool isRenderTarget = false;
		};

		class Texture
		{
		public:

			void Dispose();

			bool IsColourFormat()
			{
				if (m_Format >= VK_FORMAT_R4G4_UNORM_PACK8 && m_Format <= VK_FORMAT_B10G11R11_UFLOAT_PACK32)
					return true;

				return false;
			}

			bool IsDepthFormat()
			{
				if (m_Format == VK_FORMAT_D32_SFLOAT || m_Format == VK_FORMAT_D24_UNORM_S8_UINT)
					return true;
			}

			bool IsStencilFormat()
			{
				if (m_Format == VK_FORMAT_D24_UNORM_S8_UINT)
					return true;

				return false;
			}

		private:

			friend class Swapchain;
			friend class CommandList;
			friend class DescriptorSet;
			friend class Device;

			VkDevice m_AssociatedDevice;
			VmaAllocator m_AssociatedAllocator;

			VkImage m_Image;
			VkImageView m_ImageView;
			VmaAllocation m_Allocation;

			VkImageLayout m_Layout;
			VkFormat m_Format;

			uint32_t m_Width, m_Height, m_Depth = 1;

			bool m_SwapchainImage = false;

			bool m_InternallyManaged = false;

			void Create(const TextureDesc& desc);
		};
	}
}