#pragma once

#include <queue>
#include "VulkanInclude.h"

namespace hf
{
	namespace vulkan
	{
		// Adapted From https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/

		class DescriptorSetAllocator
		{
		public:

			struct PoolSizes
			{
				std::vector<std::pair<VkDescriptorType, float>> sizes =
				{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
				};
			};

			bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

			void Init(VkDevice device);

			void Dispose();

			void Reset();

		private:

			VkDescriptorPool GetPool();
			
			VkDevice m_Device;

			VkDescriptorPool m_CurrentPool;
			std::queue<VkDescriptorPool> m_UsedPools;
			std::queue<VkDescriptorPool> m_FreePools;

			PoolSizes m_PoolSizes;
		};
	}
}