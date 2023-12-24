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
		GetMainWindow()->SetUseDarkMode(true);


		renderer = hf::Renderer::CreateRenderer();
		renderer->RegisterWindow(GetMainWindow());



		hf::vulkan::DescriptorSetLayout layout1;
		layout1.AddUniformBuffer(hf::ShaderStage::Vertex, 0, 1);
		layout1.AddTextureSampler(hf::ShaderStage::Fragment, 1, 1);

		hf::vulkan::GraphicsPipelineDesc pipelineDesc{};
		pipelineDesc.colourTargetFormats = { renderer->GetSwapchainFormat(GetMainWindow()) };
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

		

		hf::BufferDesc bufferDesc{};
		bufferDesc.bufferType = hf::BufferType::Vertex;
		bufferDesc.cpuVisible = false;
		bufferDesc.size = sizeof(Vertex) * vertices.size();

		vertexBuffer = renderer->CreateBuffer(bufferDesc);
		vertexBuffer->Upload(vertices.data(), sizeof(Vertex) * vertices.size());

		hf::BufferDesc indexDesc{};
		indexDesc.bufferType = hf::BufferType::Index;
		indexDesc.cpuVisible = false;
		indexDesc.size = sizeof(uint16_t) * indices.size();

		indexBuffer = renderer->CreateBuffer(indexDesc);
		indexBuffer->Upload(indices.data(), sizeof(uint16_t) * indices.size());
		
		proj = glm::perspective(glm::radians(70.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);


		hf::BufferDesc uniformDesc{};
		uniformDesc.bufferType = hf::BufferType::Uniform;
		uniformDesc.cpuVisible = true;
		uniformDesc.size = sizeof(glm::mat4);

		uniformBuffer = renderer->CreateBuffer(uniformDesc);


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
		samplerState.maxAnisotropy = ((hf::RendererVk*)renderer)->m_Device.GetSupportedFeatures().maxAnis otropy;

		descriptorSet = ((hf::RendererVk*)renderer)->m_Device.AllocateDescriptorSet(layout1);
		descriptorSet.BindUniformBuffer(((hf::BufferVk*)uniformBuffer.get())->m_Buffer, 0);
		descriptorSet.BindTextureSampler(testTexture, samplerState, 1);
		descriptorSet.Write();

		

		GetMainWindow()->SetResizeCallback([&](uint32_t width, uint32_t height) 
			{
				proj = glm::perspective(glm::radians(70.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);
			});

		imgStaging.Dispose();

	
	}

	glm::mat4 proj;

	void Update(float deltaTime) override
	{

		camera.Update(deltaTime);

		glm::mat4 view = camera.GetMatrix();
		glm::mat4 vp = proj * view;

		uniformBuffer->Upload(&vp, sizeof(vp));

	}

	void Render() override
	{
		

		if (renderer->BeginFrame(GetMainWindow()))
		{

			hf::vulkan::Texture* backbuffer = ((hf::RendererVk*)renderer)->GetWindowData(GetMainWindow()).swapchain.GetSwapchainImage();


			hf::vulkan::CommandList& cmdList = ((hf::RendererVk*)renderer)->GetCurrentFrameCmdList(GetMainWindow());

			cmdList.Begin();


			hf::vulkan::Attachment attachment{};
			attachment.texture = backbuffer;
			attachment.loadOp = hf::vulkan::LoadOp::Clear;
			attachment.storeOp = hf::vulkan::StoreOp::Store;
			attachment.clearColour = { 0.3f, 0.4f, 0.9f, 0.0f };

			hf::vulkan::RenderpassInfo rpInfo{};
			rpInfo.colourAttachments = { attachment };
			cmdList.BeginRenderpass(rpInfo);



			cmdList.SetViewport(0, 0, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());
			cmdList.SetScissor(0, 0, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());

			cmdList.BindPipeline(&graphicsPipeline);

			cmdList.BindDescriptorSets({ &descriptorSet }, 0);

			cmdList.BindVertexBuffer(&((hf::BufferVk*)vertexBuffer.get())->m_Buffer, 0);
			cmdList.BindIndexBuffer(&((hf::BufferVk*)indexBuffer.get())->m_Buffer, hf::IndexType::Uint16);

			cmdList.DrawIndexed(6, 0);

			cmdList.EndRenderpass();

			cmdList.ResourceBarrier(backbuffer, hf::vulkan::ImageLayout::PresentSrc);

			cmdList.End();

			renderer->EndFrame(GetMainWindow());
		}
		
	}

	void Destroy() override
	{
		((hf::RendererVk*)renderer)->WaitIdle();

		vertexBuffer->Dispose();
		indexBuffer->Dispose();
		uniformBuffer->Dispose();
		graphicsPipeline.Dispose();
		testTexture.Dispose();


		hf::Renderer::DestroyRenderer(renderer);

	
	}

	hf::Renderer* renderer;

	FPSCamera camera;

	hf::vulkan::DescriptorSet descriptorSet;
	hf::vulkan::Texture testTexture;

	std::shared_ptr<hf::Buffer> vertexBuffer;
	std::shared_ptr<hf::Buffer> uniformBuffer;
	std::shared_ptr<hf::Buffer> indexBuffer;

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