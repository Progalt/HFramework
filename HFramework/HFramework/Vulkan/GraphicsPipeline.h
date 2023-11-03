#pragma once
#include <vector>
#include <unordered_map>
#include "../Graphics/Format.h"
#include "../Graphics/ShaderEnums.h"

namespace hf
{
	namespace vulkan
	{

	

		struct ShaderDesc
		{
			std::vector<uint32_t> bytecode;
		};

		struct VertexInput
		{
			
		};

		struct GraphicsPipelineDesc
		{
			std::unordered_map<ShaderStage, ShaderDesc> shaders = {};

			std::vector<VertexInput> vertexLayout = {};

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
		private:
		};
	}
}