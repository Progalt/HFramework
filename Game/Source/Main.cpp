#include "HFramework/HFramework.h"

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		);

		graphicsPipeline = device.RetrieveGraphicsPipeline(pipelineDesc);

		std::vector<Vertex> vertices = {
			{ -0.5f,  -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f },  
			{ 0.5f,  -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.5f,  0.5f, 0.0f ,   0.0f, 0.0f, 1.0f, 1.0f},
			{ -0.5f,  0.5f, 0.0f ,   0.0f, 1.0f, 1.0f, 1.0f}
		};
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3
		};

		hf::vulkan::BufferDesc bufferDesc{};
		bufferDesc.usage = hf::vulkan::BufferUsage::Vertex;
		bufferDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		bufferDesc.bufferSize = sizeof(Vertex) * vertices.size();

		vertexBuffer = device.CreateBuffer(bufferDesc);

		void* mapped = vertexBuffer.Map();
		memcpy(mapped, vertices.data(), bufferDesc.bufferSize);
		vertexBuffer.Unmap();

		hf::vulkan::BufferDesc indexBufferDesc{};
		indexBufferDesc.usage = hf::vulkan::BufferUsage::Index;
		indexBufferDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		indexBufferDesc.bufferSize = sizeof(uint16_t) * indices.size();

		indexBuffer = device.CreateBuffer(indexBufferDesc);

		void* mappedIdx = indexBuffer.Map();
		memcpy(mappedIdx, indices.data(), indexBufferDesc.bufferSize);
		indexBuffer.Unmap();

		
		proj = glm::perspective(glm::radians(60.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);


		hf::vulkan::BufferDesc uniformDesc{};
		uniformDesc.usage = hf::vulkan::BufferUsage::Uniform;
		uniformDesc.visibility = hf::vulkan::BufferVisibility::HostVisible;
		uniformDesc.bufferSize = sizeof(glm::mat4);

		uniformBuffer = device.CreateBuffer(uniformDesc);

		descriptorSet = device.AllocateDescriptorSet(layout1);
		descriptorSet.BindUniformBuffer(uniformBuffer, 0);
		descriptorSet.Write();

		GetMainWindow()->SetResizeCallback([&](uint32_t width, uint32_t height) 
			{
				proj = glm::perspective(glm::radians(60.0f), (float)GetMainWindow()->GetWidth() / (float)GetMainWindow()->GetHeight(), 0.01f, 100.0f);
			});
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