#pragma once
#include "VulkanInclude.h"
#include "../Graphics/ShaderEnums.h"
#include <vector>
#include "../Core/Util.h"

namespace hf
{
	namespace vulkan
	{
		class DescriptorSetLayout
		{
		public:

			DescriptorSetLayout& AddUniformBuffer(ShaderStage stage, uint32_t binding, uint32_t count)
			{
				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding = binding;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = count;

				switch (stage)
				{
				case ShaderStage::Vertex:
					layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case ShaderStage::Fragment:
					layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				case ShaderStage::Compute:
					layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
					break;
				};

				layoutBinding.pImmutableSamplers = nullptr;

				m_LayoutBindings.push_back(layoutBinding);

				return *this;
			}

			DescriptorSetLayout& AddTextureSampler(ShaderStage stage, uint32_t binding, uint32_t count)
			{
				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding = binding;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layoutBinding.descriptorCount = count;

				switch (stage)
				{
				case ShaderStage::Vertex:
					layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case ShaderStage::Fragment:
					layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				case ShaderStage::Compute:
					layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
					break;
				};

				layoutBinding.pImmutableSamplers = nullptr;

				m_LayoutBindings.push_back(layoutBinding);

				return *this;
			}


			size_t Hash()
			{
				size_t hash = 0;
				for (auto& binding : m_LayoutBindings)
					hash_combine(hash, HashBinding(binding));

				return hash;
			}
		
		private:

			friend class Device;

			size_t HashBinding(VkDescriptorSetLayoutBinding& binding)
			{
				size_t hash = 0;
				hash_combine(hash, binding.binding);
				hash_combine(hash, binding.descriptorType);
				hash_combine(hash, binding.descriptorCount);
				hash_combine(hash, binding.stageFlags);
				return hash;
			}


			std::vector< VkDescriptorSetLayoutBinding> m_LayoutBindings;
		};
	}
}