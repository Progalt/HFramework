#pragma once
#include <vector>
#include "VulkanInclude.h"
#include "../Core/Log.h"

namespace hf
{
	namespace vulkan
	{
		class FencePool
		{
		public:

			void Initialise(VkDevice device, uint32_t initialPoolSize)
			{
				m_ParentDevice = device;

				AllocateNewFreeFences(initialPoolSize);
			}

			void Dispose()
			{
				for (auto& fence : m_InUseFences)
					m_FreeFences.push_back(fence);

				for (auto& fence : m_FreeFences)
					vkDestroyFence(m_ParentDevice, fence, nullptr);
			}

			VkFence GetNewFence()
			{
				// A fence is already not in use
				if (m_FreeFences.size() > 0)
				{
					VkFence fence = m_FreeFences[0];
					vkResetFences(m_ParentDevice, 1, &fence);

					m_FreeFences.erase(m_FreeFences.begin());
					m_InUseFences.push_back(fence);

					return fence;
				}

				Log::Info("Fence Pool hit capacity, reallocating");

				// TODO: Impl
		
			
			}

			void Reclaim()
			{

				for (uint32_t i = 0; i < m_InUseFences.size(); i++)
				{
					// Get if the fence is signalled. If it is then we want to reset and add back into the free pool
					if (vkGetFenceStatus(m_ParentDevice, m_InUseFences[i]) == VK_SUCCESS)
					{
						m_FreeFences.push_back(m_InUseFences[i]);
						m_InUseFences.erase(m_InUseFences.begin() + i);

						//Log::Info("Fence reclaimed");
					}

					
				}

			}

			

		private:

			void AllocateNewFreeFences(uint32_t count)
			{
				uint32_t startSize = m_FreeFences.size();
				m_FreeFences.resize(m_FreeFences.size() + count);

				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = 0;

				for (size_t i = startSize; i < m_FreeFences.size(); i++)
				{
					if (vkCreateFence(m_ParentDevice, &fenceInfo, nullptr, &m_FreeFences[i]) != VK_SUCCESS)
					{
						Log::Fatal("Failed to create new fence");
					}
				}
			}

			VkDevice m_ParentDevice;

			std::vector<VkFence> m_FreeFences;
			std::vector<VkFence> m_InUseFences;
		};
	}
}