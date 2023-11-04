#pragma once

#include "VulkanInclude.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Texture.h"
#include <vector>

namespace hf
{
	namespace vulkan
	{
		class Device;

		class DescriptorSet
		{
		public:

			void BindUniformBuffer(Buffer& buffer, uint32_t binding, uint32_t dstArrayElement = 0, size_t offset = 0, size_t range = 0);

			void BindTextureSampler(Texture& texture, SamplerState& samplerState, uint32_t binding, uint32_t arrayElement = 0);

			void Write();

		private:

			friend class Device;
			friend class CommandList;

			Device* m_Device;

			std::vector<VkWriteDescriptorSet> m_Writes;

			std::vector< VkDescriptorBufferInfo> m_BufferInfo;
			std::vector< VkDescriptorImageInfo> m_ImageInfo;

			VkDescriptorSet m_Set;
		};
	}
}