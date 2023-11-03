#pragma once
#include "VulkanInclude.h"

namespace hf
{
	namespace vulkan
	{
		class Texture
		{
		public:

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

			VkImage m_Image;
			VkImageView m_ImageView;
			VkImageLayout m_Layout;
			VkFormat m_Format;

			uint32_t m_Width, m_Height;

			bool m_SwapchainImage = false;

			bool m_InternallyManaged;
		};
	}
}