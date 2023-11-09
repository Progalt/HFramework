#include "HFramework/HFramework.h"

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "HFramework/Graphics/Renderer.h"
#include "HFramework/Graphics/Vulkan/RendererVk.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "FPSCamera.h"

static std::vector<uint8_t> readFile(const std::string& filename) 
{
	std::ifstream instream(filename, std::ios::in | std::ios::binary);

	if (!instream.is_open())
	{
		hf::Log::Error("Failed to open file");
	}

	std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
	return data;
}

class Game : public hf::Application
{
public:

	struct Vertex
	{
		float x, y, z;
		float r, g, b, a;
		float u, v;
	};

	void Start() override
	{
		renderer = hf::Renderer::CreateRenderer(GetMainWindow());


		hf::vulkan::DescriptorSetLayout layout1;
		layout1.AddUniformBuffer(hf::ShaderStage::Vertex, 0, 1);
		layout1.AddTextureSampler(hf::ShaderStage::Fragment, 1, 1);

		hf::vulkan::GraphicsPipelineDesc pipelineDesc{};
		pipelineDesc.colourTargetFormats = { ((hf::RendererVk*)renderer)->m_Swapchain.GetImageFormat() };
		pipelineDesc.shaders[hf::ShaderStage::Vertex].bytecode = readFile("Assets/Shaders/base.vert.spv");
		pipelineDesc.shaders[hf::ShaderStage::Fragment].bytecode = readFile("Assets/Shaders/base.frag.spv");
		pipelineDesc.topologyMode = hf::TopologyMode::Triangles;
		pipelineDesc.cullMode = hf::CullMode::None;
		pipelineDesc.setLayouts = { layout1 };

		pipelineDesc.vertexLayout.push_back(
			hf::vulkan::VertexInput(0, sizeof(Vertex))
			.AddAttribute(hf::vulkan::VertexAttribute(0, hf::Format::RGB32F, 0))
			.AddAttribute(hf::vulkan::VertexAttribute(1, hf::Format::RGBA32F, 3 * sizeof(float)))
			.AddAttribute(hf::vulkan::VertexAttribute(2, hf::Format::RG32F, 7 * sizeof(float)))
		);

		graphicsPipeline = ((hf::RendererVk*)renderer)->m_Device.RetrieveGraphicsPipeline(pipelineDesc);

		std::vector<Vertex> vertices = {
			{ -0.5f,  -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 0.0f },
			{ 0.5f,  -0.5f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 0.0f },
			{ 0.5f,  0.5f,0.0f,   1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f },
			{ -0.5f,  0.5f,0.0f,   1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 1.0f }
		};
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3
		};

		

		hf::vulkan::BufferDesc stagingBufferDesc{};
		stagingBufferDesc.usage = hf::vulkan::BufferUsage::TransferSrc;
		stagingBufferDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		stagingBufferDesc.bufferSize = sizeof(Vertex) * vertices.size() + sizeof(uint16_t) * indices.size();

		hf::vulkan::Buffer staging = ((hf::RendererVk*)renderer)->m_Device.CreateBuffer(stagingBufferDesc);

		hf::vulkan::BufferDesc bufferDesc{};
		bufferDesc.usage = hf::vulkan::BufferUsage::Vertex | hf::vulkan::BufferUsage::TransferDst;
		bufferDesc.visibility = hf::vulkan::BufferVisibility::Device;
		bufferDesc.bufferSize = sizeof(Vertex) * vertices.size();

		vertexBuffer = ((hf::RendererVk*)renderer)->m_Device.CreateBuffer(bufferDesc);


		hf::vulkan::BufferDesc indexBufferDesc{};
		indexBufferDesc.usage = hf::vulkan::BufferUsage::Index | hf::vulkan::BufferUsage::TransferDst;
		indexBufferDesc.visibility = hf::vulkan::BufferVisibility::Device;
		indexBufferDesc.bufferSize = sizeof(uint16_t) * indices.size();

		indexBuffer = ((hf::RendererVk*)renderer)->m_Device.CreateBuffer(indexBufferDesc);

		void* mapped = staging.Map();
		char* mappedC = (char*)mapped;
		memcpy(mappedC, vertices.data(), bufferDesc.bufferSize);
		memcpy(mappedC + bufferDesc.bufferSize, indices.data(), indexBufferDesc.bufferSize);
		staging.Unmap();

		((hf::RendererVk*)renderer)->m_Device.ExecuteSingleUsageCommandList(hf::vulkan::Queue::Graphics, [&](hf::vulkan::CommandList& cmd)
			{
				cmd.CopyBuffer(&staging, &vertexBuffer, sizeof(Vertex) * vertices.size());
				cmd.CopyBuffer(&staging, &indexBuffer, sizeof(uint16_t) * indices.size(), sizeof(Vertex) * vertices.size());
			});

		
		proj = glm::perspective(glm::radians(70.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);


		hf::vulkan::BufferDesc uniformDesc{};
		uniformDesc.usage = hf::vulkan::BufferUsage::Uniform;
		uniformDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		uniformDesc.bufferSize = sizeof(glm::mat4);

		uniformBuffer = ((hf::RendererVk*)renderer)->m_Device.CreateBuffer(uniformDesc);


		/* texture load */

		int w, h, c;
		uint8_t* pixels = stbi_load("Assets/512.png", &w, &h, &c, STBI_rgb_alpha);

		if (!pixels)
			hf::Log::Fatal("Failed to load image");

		hf::vulkan::TextureDesc textureDesc{};
		textureDesc.width = w;
		textureDesc.height = h;
		textureDesc.format = hf::Format::RGBA8_SRGB;
		textureDesc.type = hf::vulkan::TextureType::Flat2D;

		testTexture = ((hf::RendererVk*)renderer)->m_Device.CreateTexture(textureDesc);

		hf::vulkan::BufferDesc bufferDesc2{};
		bufferDesc2.usage = hf::vulkan::BufferUsage::TransferSrc;
		bufferDesc2.visibility = hf::vulkan::BufferVisibility::HostVisible;
		bufferDesc2.bufferSize = w * h * 4;

		hf::vulkan::Buffer imgStaging = ((hf::RendererVk*)renderer)->m_Device.CreateBuffer(bufferDesc2);

		void* imgMem = imgStaging.Map();
		memcpy(imgMem, pixels, bufferDesc2.bufferSize);
		imgStaging.Unmap();

		((hf::RendererVk*)renderer)->m_Device.ExecuteSingleUsageCommandList(hf::vulkan::Queue::Graphics, [&](hf::vulkan::CommandList& cmd)
			{
				cmd.ResourceBarrier(&testTexture, hf::vulkan::ImageLayout::TransferDst);
				hf::vulkan::BufferImageCopy imgCopy{};
				imgCopy.extent.width = w;
				imgCopy.extent.height = h;
				cmd.CopyBufferToTexture(&imgStaging, &testTexture, imgCopy);
				cmd.ResourceBarrier(&testTexture, hf::vulkan::ImageLayout::ShaderReadOnlyOptimal);
			});

		hf::vulkan::SamplerState samplerState;
		samplerState.min = hf::FilterMode::Linear;
		samplerState.mag = hf::FilterMode::Linear;
		samplerState.maxAnisotropy = ((hf::RendererVk*)renderer)->m_Device.GetSupportedFeatures().maxAnisotropy;

		descriptorSet = ((hf::RendererVk*)renderer)->m_Device.AllocateDescriptorSet(layout1);
		descriptorSet.BindUniformBuffer(uniformBuffer, 0);
		descriptorSet.BindTextureSampler(testTexture, samplerState, 1);
		descriptorSet.Write();

		

		GetMainWindow()->SetResizeCallback([&](uint32_t width, uint32_t height) 
			{
				proj = glm::perspective(glm::radians(70.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);
			});

		staging.Dispose();
		imgStaging.Dispose();

		nestedCmds = ((hf::RendererVk*)renderer)->m_Device.AllocateCommandLists(hf::vulkan::Queue::Graphics, hf::vulkan::CommandListType::Secondary, ((hf::RendererVk*)renderer)->m_Swapchain.GetImageCount());
	}

	glm::mat4 proj;

	void Update(float deltaTime) override
	{
		static float total = 0.0f;
		total += deltaTime * 0.5f;

		glm::vec3 pos = { sin(total) * 2.0f, 1.0f, cos(total) * 2.0f };

		camera.Update(deltaTime);

		glm::mat4 view = camera.GetMatrix();
		glm::mat4 vp = proj * view;

		void* uboMapped = uniformBuffer.Map();
		memcpy(uboMapped, &vp, sizeof(vp));
		uniformBuffer.Unmap();


	}

	void Render() override
	{
		
		
		if (!renderer->BeginFrame())
			return;

		hf::vulkan::CommandList& nestedCmd = nestedCmds[((hf::RendererVk*)renderer)->m_CurrentFrameIndex];

		hf::vulkan::Texture* backbuffer = ((hf::RendererVk*)renderer)->m_Swapchain.GetSwapchainImage();

		hf::vulkan::Attachment attachment{};
		attachment.texture = backbuffer;
		attachment.loadOp = hf::vulkan::LoadOp::Clear;
		attachment.storeOp = hf::vulkan::StoreOp::Store;
		attachment.clearColour = { 0.3f, 0.4f, 0.9f, 0.0f };

		hf::vulkan::RenderpassInfo rpInfo{};
		rpInfo.colourAttachments = { attachment };
		rpInfo.useSecondaryListsForRendering = true;

		nestedCmd.Begin(&rpInfo);

		nestedCmd.SetViewport(0, 0, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());
		nestedCmd.SetScissor(0, 0, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());

		nestedCmd.BindPipeline(&graphicsPipeline);

		nestedCmd.BindDescriptorSets({ &descriptorSet }, 0);

		nestedCmd.BindVertexBuffer(&vertexBuffer, 0);
		nestedCmd.BindIndexBuffer(&indexBuffer, hf::IndexType::Uint16);

		nestedCmd.DrawIndexed(6, 0);

		nestedCmd.End();

		hf::vulkan::CommandList& cmdList = ((hf::RendererVk*)renderer)->GetCurrentFrameCmdList();

		cmdList.Begin();



		cmdList.BeginRenderpass(rpInfo);


		cmdList.ExecuteCommandList(&nestedCmd);

		cmdList.EndRenderpass();

		cmdList.ResourceBarrier(backbuffer, hf::vulkan::ImageLayout::PresentSrc);

		cmdList.End();

		renderer->EndFrame();

	}

	void Destroy() override
	{
		((hf::RendererVk*)renderer)->WaitIdle();

		vertexBuffer.Dispose();
		indexBuffer.Dispose();
		uniformBuffer.Dispose();
		graphicsPipeline.Dispose();
		testTexture.Dispose();


		hf::Renderer::DestroyRenderer(renderer);

	
	}

	hf::Renderer* renderer;

	FPSCamera camera;

	hf::vulkan::Buffer vertexBuffer;
	hf::vulkan::Buffer indexBuffer;
	hf::vulkan::Buffer uniformBuffer;
	hf::vulkan::DescriptorSet descriptorSet;
	hf::vulkan::Texture testTexture;

	std::vector<hf::vulkan::CommandList> nestedCmds;

	hf::vulkan::GraphicsPipeline graphicsPipeline;

private:
};

int main(int argc, char* argv[])
{
	hf::StartInfo startInfo{};
	startInfo.title = "HFramework";
	startInfo.width = 1280;
	startInfo.height = 720;

	Game game;
	game.Run(startInfo);

	return 0;
}