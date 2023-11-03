

#include "DescriptorSetAllocator.h"
#include "../Core/Log.h"

namespace hf
{
	namespace vulkan
	{
		VkDescriptorPool CreatePool(VkDevice device, const DescriptorSetAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
		{
			std::vector<VkDescriptorPoolSize> sizes;
			sizes.reserve(poolSizes.sizes.size());
			for (auto sz : poolSizes.sizes) {
				sizes.push_back({ sz.first, uint32_t(sz.second * count) });
			}
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = flags;
			pool_info.maxSets = count;
			pool_info.poolSizeCount = (uint32_t)sizes.size();
			pool_info.pPoolSizes = sizes.data();

			VkDescriptorPool descriptorPool;
			vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

			Log::Info("Created New Descriptor Pool");

			return descriptorPool;
		}


		bool DescriptorSetAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
		{
			if (!m_CurrentPool)
			{
				m_CurrentPool = GetPool();
				m_UsedPools.push(m_CurrentPool);
			}

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;

			allocInfo.pSetLayouts = &layout;
			allocInfo.descriptorPool = m_CurrentPool;
			allocInfo.descriptorSetCount = 1;

			VkResult allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, set);
			bool needReallocate = false;

			switch (allocResult) {
			case VK_SUCCESS:
				return true;
			case VK_ERROR_FRAGMENTED_POOL:
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				needReallocate = true;
				break;
			default:
				return false;
			}

			if (needReallocate)
			{
				m_CurrentPool = GetPool();
				m_UsedPools.push(m_CurrentPool);

				allocInfo.descriptorPool = m_CurrentPool;

				allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, set);

				if (allocResult == VK_SUCCESS) {
					return true;
				}
			}

			return false;
		}

		void DescriptorSetAllocator::Init(VkDevice device)
		{
			m_Device = device;
		}

		void DescriptorSetAllocator::Dispose()
		{
			for (uint32_t i = 0; i < m_FreePools.size(); i++)
			{
				VkDescriptorPool pool = m_FreePools.front();
				m_FreePools.pop();

				vkDestroyDescriptorPool(m_Device, pool, nullptr);
			}

			for (uint32_t i = 0; i < m_UsedPools.size(); i++)
			{
				VkDescriptorPool pool = m_UsedPools.front();
				m_UsedPools.pop();

				vkDestroyDescriptorPool(m_Device, pool, nullptr);
			}
		}

		void DescriptorSetAllocator::Reset()
		{
			for (uint32_t i = 0; i < m_UsedPools.size(); i++)
			{
				VkDescriptorPool pool = m_UsedPools.front();
				m_UsedPools.pop();
				vkResetDescriptorPool(m_Device, pool, 0);
				m_FreePools.push(pool);
			}

			m_CurrentPool = VK_NULL_HANDLE;
		}

		VkDescriptorPool DescriptorSetAllocator::GetPool()
		{
			// See if we have a pool available 
			if (!m_FreePools.empty())
			{
				VkDescriptorPool pool = m_FreePools.front();
				m_FreePools.pop();

				return pool;
			}

			return CreatePool(m_Device, m_PoolSizes, 1000, 0);
		}
	}
}