#include "HFramework/HFramework.h"

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
		hf::vulkan::DeviceCreateInfo deviceInfo{};
		deviceInfo.validationLayers = true;
		deviceInfo.window = GetMainWindow();

		device.Create(deviceInfo);

		swapchain = device.CreateSwapchain();

		cmdLists = device.AllocateCommandLists(hf::vulkan::Queue::Graphics, hf::vulkan::CommandListType::Primary, swapchain.GetImageCount());

		workFinished = device.CreateSemaphores(swapchain.GetImageCount());
		imageAvailable = device.CreateSemaphores(swapchain.GetImageCount());

		hf::vulkan::DescriptorSetLayout layout1;
		layout1.AddUniformBuffer(hf::ShaderStage::Vertex, 0, 1);
		layout1.AddTextureSampler(hf::ShaderStage::Fragment, 1, 1);

		hf::vulkan::GraphicsPipelineDesc pipelineDesc{};
		pipelineDesc.colourTargetFormats = { swapchain.GetImageFormat() };
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

		graphicsPipeline = device.RetrieveGraphicsPipeline(pipelineDesc);

		std::vector<Vertex> vertices = {
			{ -0.5f,  -0.5f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 0.0f },
			{ 0.5f,  -0.5f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 0.0f },
			{ 0.5f,  0.5f, 0.0f ,   1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f ,   1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 1.0f }
		};
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3
		};

		

		hf::vulkan::BufferDesc stagingBufferDesc{};
		stagingBufferDesc.usage = hf::vulkan::BufferUsage::TransferSrc;
		stagingBufferDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		stagingBufferDesc.bufferSize = sizeof(Vertex) * vertices.size() + sizeof(uint16_t) * indices.size();

		hf::vulkan::Buffer staging = device.CreateBuffer(stagingBufferDesc);

		hf::vulkan::BufferDesc bufferDesc{};
		bufferDesc.usage = hf::vulkan::BufferUsage::Vertex | hf::vulkan::BufferUsage::TransferDst;
		bufferDesc.visibility = hf::vulkan::BufferVisibility::Device;
		bufferDesc.bufferSize = sizeof(Vertex) * vertices.size();

		vertexBuffer = device.CreateBuffer(bufferDesc);


		hf::vulkan::BufferDesc indexBufferDesc{};
		indexBufferDesc.usage = hf::vulkan::BufferUsage::Index | hf::vulkan::BufferUsage::TransferDst;
		indexBufferDesc.visibility = hf::vulkan::BufferVisibility::Device;
		indexBufferDesc.bufferSize = sizeof(uint16_t) * indices.size();

		indexBuffer = device.CreateBuffer(indexBufferDesc);

		void* mapped = staging.Map();
		char* mappedC = (char*)mapped;
		memcpy(mappedC, vertices.data(), bufferDesc.bufferSize);
		memcpy(mappedC + bufferDesc.bufferSize, indices.data(), indexBufferDesc.bufferSize);
		staging.Unmap();

		device.ExecuteSingleUsageCommandList(hf::vulkan::Queue::Graphics, [&](hf::vulkan::CommandList& cmd)
			{
				cmd.CopyBuffer(&staging, &vertexBuffer, sizeof(Vertex) * vertices.size());
				cmd.CopyBuffer(&staging, &indexBuffer, sizeof(uint16_t) * indices.size(), sizeof(Vertex) * vertices.size());
			});

		
		proj = glm::perspective(glm::radians(70.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);


		hf::vulkan::BufferDesc uniformDesc{};
		uniformDesc.usage = hf::vulkan::BufferUsage::Uniform;
		uniformDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		uniformDesc.bufferSize = sizeof(glm::mat4);

		uniformBuffer = device.CreateBuffer(uniformDesc);


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

		testTexture = device.CreateTexture(textureDesc);

		hf::vulkan::BufferDesc bufferDesc2{};
		bufferDesc2.usage = hf::vulkan::BufferUsage::TransferSrc;
		bufferDesc2.visibility = hf::vulkan::BufferVisibility::HostVisible;
		bufferDesc2.bufferSize = w * h * 4;

		hf::vulkan::Buffer imgStaging = device.CreateBuffer(bufferDesc2);

		void* imgMem = imgStaging.Map();
		memcpy(imgMem, pixels, bufferDesc2.bufferSize);
		imgStaging.Unmap();

		device.ExecuteSingleUsageCommandList(hf::vulkan::Queue::Graphics, [&](hf::vulkan::CommandList& cmd)
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
		samplerState.maxAnisotropy = device.GetSupportedFeatures().maxAnisotropy;

		descriptorSet = device.AllocateDescriptorSet(layout1);
		descriptorSet.BindUniformBuffer(uniformBuffer, 0);
		descriptorSet.BindTextureSampler(testTexture, samplerState, 1);
		descriptorSet.Write();

		

		GetMainWindow()->SetResizeCallback([&](uint32_t width, uint32_t height) 
			{
				proj = glm::perspective(glm::radians(60.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);
			});

		staging.Dispose();
		imgStaging.Dispose();
	}

	glm::mat4 proj;

	void Update(float deltaTime) override
	{
		static float total = 0.0f;
		total += deltaTime;

		glm::vec3 pos = { sin(total) * 2.0f, 1.0f, cos(total) * 2.0f };

		glm::mat4 view = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 vp = proj * view;

		void* uboMapped = uniformBuffer.Map();
		memcpy(uboMapped, &vp, sizeof(vp));
		uniformBuffer.Unmap();

	}

	void Render() override
	{
		uint32_t currentImageIdx = swapchain.GetCurrentImageIndex();

		if (!swapchain.AquireNextFrame(&imageAvailable[currentImageIdx]))
			return;
		


		hf::vulkan::CommandList& cmdList = cmdLists[currentImageIdx];

		cmdList.Begin();

		hf::vulkan::Texture* backbuffer = swapchain.GetSwapchainImage();

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

		cmdList.BindVertexBuffer(&vertexBuffer, 0);
		cmdList.BindIndexBuffer(&indexBuffer, hf::vulkan::IndexType::Uint16);

		cmdList.DrawIndexed(6, 0); 

		cmdList.EndRenderpass();

		cmdList.ResourceBarrier(backbuffer, hf::vulkan::ImageLayout::PresentSrc);

		cmdList.End();

		device.QueueSubmit(hf::vulkan::Queue::Graphics, { &cmdList }, &imageAvailable[currentImageIdx], &workFinished[currentImageIdx]);

		swapchain.Present(&workFinished[currentImageIdx]);
	}

	void Destroy() override
	{
		device.WaitIdle();

		vertexBuffer.Dispose();
		indexBuffer.Dispose();
		uniformBuffer.Dispose();
		graphicsPipeline.Dispose();
		testTexture.Dispose();

		for (auto& semaphore : imageAvailable)
			semaphore.Dispose();

		for (auto& semaphore : workFinished)
			semaphore.Dispose();

		swapchain.Dispose();
		device.Dispose();
	}


	hf::vulkan::Device device;
	hf::vulkan::Swapchain swapchain;

	std::vector<hf::vulkan::CommandList> cmdLists;
	std::vector<hf::vulkan::Semaphore> workFinished;
	std::vector<hf::vulkan::Semaphore> imageAvailable;

	hf::vulkan::Buffer vertexBuffer;
	hf::vulkan::Buffer indexBuffer;
	hf::vulkan::Buffer uniformBuffer;
	hf::vulkan::DescriptorSet descriptorSet;
	hf::vulkan::Texture testTexture;

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