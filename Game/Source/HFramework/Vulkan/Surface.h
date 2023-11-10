#pragma once
#include "VulkanInclude.h"
#include "../Core/Window.h"

namespace hf
{
	namespace vulkan
	{
		class Surface
		{
		public:

			void Dispose()
			{
				vkDestroySurfaceKHR(m_AssociatedInstance, m_Surface, nullptr);
			}

		private:

			friend class Device;
			friend class Swapchain;

			Window* m_AssociatedWindow;

			VkSurfaceKHR m_Surface;
			bool m_SupportsPresent;
			uint32_t m_PresentQueueIndex = UINT32_MAX;

			VkPhysicalDevice m_AssociatedPhysicalDevice;

			VkInstance m_AssociatedInstance;
		};
	}
}