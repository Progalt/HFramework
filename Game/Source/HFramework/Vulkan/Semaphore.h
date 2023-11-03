#pragma once
#include "VulkanInclude.h"
#include <vector>

namespace hf
{
	namespace vulkan
	{
		class Semaphore
		{
		public:

			void Dispose()
			{
				vkDestroySemaphore(m_ParentDevice, m_Semaphore, nullptr);
			}

		private:

			friend class Device;
			friend class Swapchain;

			VkDevice m_ParentDevice;

			VkSemaphore m_Semaphore;
		};
	}
}