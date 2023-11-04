
#include "DescriptorSet.h"
#include "../Core/Log.h"
#include "Device.h"

namespace hf
{
	namespace vulkan
	{
		void DescriptorSet::BindUniformBuffer(Buffer& buffer, uint32_t binding, uint32_t dstArrayElement, size_t offset, size_t range)
		{


			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer.m_Buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = (range == 0) ? VK_WHOLE_SIZE : range;

			m_BufferInfo.push_back(bufferInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_Set;
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = dstArrayElement;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &m_BufferInfo[m_BufferInfo.size() - 1];
			descriptorWrite.pImageInfo = nullptr; 
			descriptorWrite.pTexelBufferView = nullptr; 

			m_Writes.push_back(descriptorWrite);
		}

		void DescriptorSet::BindTextureSampler(Texture& texture, SamplerState& samplerState, uint32_t binding, uint32_t arrayElement )
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// Set to use shader read only optimal 
			imageInfo.imageView = texture.m_ImageView;
			imageInfo.sampler = m_Device->GetSampler(samplerState);

			m_ImageInfo.push_back(imageInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_Set;
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = arrayElement;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = &m_ImageInfo[m_ImageInfo.size() - 1];
			descriptorWrite.pTexelBufferView = nullptr;

			m_Writes.push_back(descriptorWrite);
		}

		void DescriptorSet::Write()
		{
			vkUpdateDescriptorSets(m_Device->m_Device, m_Writes.size(), m_Writes.data(), 0, nullptr);

			m_BufferInfo.clear();
			m_ImageInfo.clear();
		}
	}
}