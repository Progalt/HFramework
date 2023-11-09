
#include "GraphicsPipeline.h"
#include "VulkanInclude.h"
#include "../Core/Log.h"
#include "FormatConvert.h"

namespace hf
{
	namespace vulkan
	{
		VkShaderModule createShaderModule(VkDevice device, const std::vector<uint8_t>& code)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				Log::Error("failed to create shader module!");
			}

			return shaderModule;
		}

		void GraphicsPipeline::Dispose()
		{
			vkDestroyPipelineLayout(m_CachedDevice, m_Layout, nullptr);
			vkDestroyPipeline(m_CachedDevice, m_Pipeline, nullptr);
		}

		void GraphicsPipeline::Create(VkDevice device, const GraphicsPipelineDesc& desc, std::vector<VkDescriptorSetLayout> setLayouts)
		{
			m_CachedDevice = device;

			std::vector<VkShaderModule> shaderModules;

			std::vector< VkPipelineShaderStageCreateInfo> shaderStages;

			if (desc.shaders.find(ShaderStage::Vertex) != desc.shaders.end())
			{
				// Has Vertex Stage

				VkShaderModule vertexModule = createShaderModule(device, desc.shaders.at(ShaderStage::Vertex).bytecode);

				VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
				vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertShaderStageInfo.module = vertexModule;
				vertShaderStageInfo.pName = "main";

				shaderModules.push_back(vertexModule);

				shaderStages.push_back(vertShaderStageInfo);

			}

			if (desc.shaders.find(ShaderStage::Fragment) != desc.shaders.end())
			{
				// Has Fragment Stage

				VkShaderModule fragModule = createShaderModule(device, desc.shaders.at(ShaderStage::Fragment).bytecode);

				VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
				fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragShaderStageInfo.module = fragModule;
				fragShaderStageInfo.pName = "main";

				shaderModules.push_back(fragModule);

				shaderStages.push_back(fragShaderStageInfo);

			}

			if (shaderStages.size() == 0)
				Log::Warn("No Shaders compiled for pipeline this may be unwanted.");

			std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			// --- SETUP VERTEX INPUT INFO ---

			std::vector<VkVertexInputBindingDescription> inputBindingDescs{};
			std::vector< VkVertexInputAttributeDescription> attributeDescs{};

			for (auto& binding : desc.vertexLayout)
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = binding.binding;
				bindingDescription.stride = binding.stride;
				bindingDescription.inputRate = (binding.inputRate == InputRate::Vertex) ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

				inputBindingDescs.push_back(bindingDescription);

				for (auto& attr : binding.attributes)
				{
					VkVertexInputAttributeDescription attrDesc{};

					attrDesc.binding = binding.binding;
					attrDesc.location = attr.location;
					attrDesc.offset = attr.offset;
					attrDesc.format = FormatTable[(int)attr.format];

					attributeDescs.push_back(attrDesc);
				}
			}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = inputBindingDescs.size();
			vertexInputInfo.pVertexBindingDescriptions = inputBindingDescs.data();
			vertexInputInfo.vertexAttributeDescriptionCount = attributeDescs.size();
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

			// -------------------------------------

			// --- SETUP IA INFO ---

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

			switch (desc.topologyMode)
			{
			case TopologyMode::Triangles:
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				break;
			case TopologyMode::Lines:
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				break;
			}

			inputAssembly.primitiveRestartEnable = VK_FALSE;

			// -------------------------------------

			// --- SETUP VIEWPORT STATE --- 

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			// -------------------------------------

			// --- SETUP RASTERIZER ---

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			switch (desc.cullMode)
			{
			case CullMode::Front:
				rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
				break;
			case CullMode::Back:
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
				break;
			default:
				rasterizer.cullMode = VK_CULL_MODE_NONE;
				break;
			}
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasSlopeFactor = 0.0f;

			// -------------------------------------

			// --- SETUP MULTISAMPLING INFO --- 

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable = VK_FALSE;

			// -------------------------------------

			// --- SETUP COLOUR BLENDING INFO ---

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			// -------------------------------------

			// --- SETUP PIPELINE RENDERING INFO ---

			VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
			pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
			
			std::vector<VkFormat> colourFormats{};

			for (auto& attach : desc.colourTargetFormats)
				colourFormats.push_back(FormatTable[(int)attach]);

			pipelineRenderingInfo.colorAttachmentCount = colourFormats.size();
			pipelineRenderingInfo.pColorAttachmentFormats = colourFormats.data();

			if (desc.depthTargetFormat != Format::None)
			{
				pipelineRenderingInfo.depthAttachmentFormat = FormatTable[(int)desc.depthTargetFormat];
			}

			// -------------------------------------

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = setLayouts.size();
			pipelineLayoutInfo.pSetLayouts = setLayouts.data();

			std::vector<VkPushConstantRange> pushConstants{};

			for (auto& range : desc.pushConstantRanges)
			{
				VkPushConstantRange r;
				r.size = range.second.size;
				r.offset = range.second.offset;

				switch (range.first)
				{
				case ShaderStage::Vertex:
					r.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case ShaderStage::Fragment:
					r.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				}

				pushConstants.push_back(r);
			}

			pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
			pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
			{
				Log::Error("failed to create pipeline layout!");
			}

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr; 
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = m_Layout;
			pipelineInfo.renderPass = nullptr;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;
			pipelineInfo.pNext = &pipelineRenderingInfo;

			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) 
			{
				Log::Error("failed to create graphics pipeline!");
			}


			for (auto& module : shaderModules)
				vkDestroyShaderModule(device, module, nullptr);

			Log::Info("Successfully Created Graphics Pipeline");
		}
	}
}