#pragma once
#include <vector>
#include <unordered_map>
#include "../Graphics/Format.h"
#include "../Graphics/ShaderEnums.h"
#include "VulkanInclude.h"
#include "DescriptorSetLayout.h"

namespace hf
{
	namespace vulkan
	{

		struct ShaderDesc
		{
			std::vector<uint8_t> bytecode;
		};

		struct VertexAttribute
		{
			VertexAttribute() : location(0), format(Format::None), offset(0) { }
			VertexAttribute(uint32_t location, Format format, uint32_t offset) : location(location), format(format), offset(offset) { }

			uint32_t location;
			Format format;
			uint32_t offset;
		};

		struct VertexInput
		{
			VertexInput() : binding(0), stride(0) { }
			VertexInput(uint32_t binding, uint32_t stride) : binding(binding), stride(stride) { }

			VertexInput& AddAttribute(VertexAttribute attr)
			{
				attributes.push_back(attr);

				return *this;
			}

			uint32_t binding;
			uint32_t stride;
			std::vector<VertexAttribute> attributes;
		};

		struct GraphicsPipelineDesc
		{
			std::unordered_map<ShaderStage, ShaderDesc> shaders = {};

			std::vector<VertexInput> vertexLayout = {};

			std::vector<DescriptorSetLayout> setLayouts = {};

			std::vector<Format> colourTargetFormats;
			Format depthTargetFormat = Format::None;

			TopologyMode topologyMode = TopologyMode::Triangles;
			CullMode cullMode = CullMode::None;

			bool depthTest = true;
			bool depthWrite = true;

		};

		class GraphicsPipeline
		{
		public:

			void Dispose();

		private:

			friend class Device;
			friend class CommandList;

			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;

			VkDevice m_CachedDevice;

			void Create(VkDevice device, const GraphicsPipelineDesc& desc, std::vector<VkDescriptorSetLayout> setLayouts);
		};
	}
}