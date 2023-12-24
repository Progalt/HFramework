
#include "RendererVk.h"

namespace hf
{
	void RendererVk::Init()
	{
		hf::vulkan::DeviceCreateInfo deviceInfo{};
#ifdef _DEBUG
		deviceInfo.validationLayers = true;
#else 
		deviceInfo.validationLayers = false;
#endif

		m_Device.Create(deviceInfo);

		// Create the staging buffer

		vulkan::BufferDesc stagingDesc{};
		stagingDesc.usage = vulkan::BufferUsage::TransferSrc;
		stagingDesc.visibility = vulkan::BufferVisibility::HostVisible;
		stagingDesc.bufferSize = m_StagingBuffer.size;

		m_StagingBuffer.buffer = m_Device.CreateBuffer(stagingDesc);
		m_StagingBuffer.m_Mapped = m_StagingBuffer.buffer.Map();
		m_StagingBuffer.semaphore = m_Device.CreateSemaphores(1)[0];
	}

	void RendererVk::Destroy()
	{

		m_StagingBuffer.buffer.Dispose();
		m_StagingBuffer.semaphore.Dispose();
		
		for (auto& [wnd, data] : m_WindowData)
		{
			data.swapchain.Dispose();
			data.surface.Dispose();
			

			for (auto& semaphore : data.imageAvailable)
				semaphore.Dispose();

			for (auto& semaphore : data.workFinished)
				semaphore.Dispose();
		}

		m_Device.Dispose();
	}



	hf::vulkan::CommandList& RendererVk::GetCurrentFrameCmdList(Window* window)
	{
		WindowData& windowData = m_WindowData[window];

		return windowData.baseCommandLists[windowData.currentFrameIndex];
	}


	void RendererVk::RegisterWindow(Window* window)
	{
		m_WindowData[window];
		WindowData& windowData = m_WindowData[window];

		windowData.surface = m_Device.CreateSurface(window);

		windowData.swapchain = m_Device.CreateSwapchain(&windowData.surface);

		windowData.baseCommandLists = m_Device.AllocateCommandLists(hf::vulkan::Queue::Graphics, hf::vulkan::CommandListType::Primary, windowData.swapchain.GetImageCount());
		windowData.imageAvailable = m_Device.CreateSemaphores(windowData.swapchain.GetImageCount());
		windowData.workFinished = m_Device.CreateSemaphores(windowData.swapchain.GetImageCount());
	}

	Format RendererVk::GetSwapchainFormat(Window* window)
	{
		if (m_WindowData.find(window) != m_WindowData.end())
		{
			return m_WindowData[window].swapchain.GetImageFormat();
		}

		return Format::None;
	}

	bool RendererVk::BeginFrame(Window* window)
	{
		if (!window->IsOpen())
			return false;

		// Upload Using staging

		if (m_StagingBuffer.dataUploaded)
		{
			m_Device.ExecuteSingleUsageCommandList(hf::vulkan::Queue::Graphics, [&](hf::vulkan::CommandList& cmdList)
				{

					while (!m_CopyData.empty())
					{
						CopyData& data = m_CopyData.front();

						switch (data.op)
						{
						case CopyData::CopyOp::Buffer:

							cmdList.CopyBuffer(&m_StagingBuffer.buffer, data.buffer, data.size, data.stagingOffset);

							break;
						case CopyData::CopyOp::Texture: 
							break;
						}

						m_CopyData.pop();
					}

					m_StagingBuffer.offset = 0;

				}, &m_StagingBuffer.semaphore);
		}
		

		WindowData& windowData = m_WindowData[window];

		windowData.currentFrameIndex = windowData.swapchain.GetCurrentImageIndex();

		if (!windowData.swapchain.AquireNextFrame(&windowData.imageAvailable[windowData.currentFrameIndex]))
			return false;

		return true;

	}

	void RendererVk::EndFrame(Window* window)
	{
		WindowData& windowData = m_WindowData[window];

		std::vector<vulkan::Semaphore*> wait;

		wait.push_back(&windowData.imageAvailable[windowData.currentFrameIndex]);

		// If we have uploaded something using the staging buffer
		// We want to wait for that to finish execution before beginning execution of this command list
		if (m_StagingBuffer.dataUploaded)
		{
			wait.push_back(&m_StagingBuffer.semaphore);
			m_StagingBuffer.dataUploaded = false;
		}

		m_Device.QueueSubmit(hf::vulkan::Queue::Graphics, { &GetCurrentFrameCmdList(window) }, wait, &windowData.workFinished[windowData.currentFrameIndex]);

		windowData.swapchain.Present(&windowData.workFinished[windowData.currentFrameIndex]);
	}
	 
	void RendererVk::WaitIdle()
	{
		m_Device.WaitIdle();
	}

	std::shared_ptr<Buffer> RendererVk::CreateBuffer(const BufferDesc& desc)
	{
		std::shared_ptr<BufferVk> buf = std::make_shared<BufferVk>();

		vulkan::BufferDesc bufDesc{};
		bufDesc.bufferSize = desc.size;
		bufDesc.visibility = desc.cpuVisible ? vulkan::BufferVisibility::HostVisible : vulkan::BufferVisibility::Device;

		switch (desc.bufferType)
		{
		case BufferType::Vertex:
			bufDesc.usage = vulkan::BufferUsage::Vertex;
			break;
		case BufferType::Uniform:
			bufDesc.usage = vulkan::BufferUsage::Uniform;
			break;
		case BufferType::Index:
			bufDesc.usage = vulkan::BufferUsage::Index;
			break;
		}

		// if its device only we need to have set it as a transfer destination 
		if (bufDesc.visibility == vulkan::BufferVisibility::Device)
			bufDesc.usage = bufDesc.usage | vulkan::BufferUsage::TransferDst;

		buf->m_Buffer = m_Device.CreateBuffer(bufDesc);

		buf->m_CpuVisible = desc.cpuVisible;
		if (buf->m_CpuVisible)
			buf->m_MappedBuffer = buf->m_Buffer.Map();

		buf->m_Renderer = this;

		return buf;
	}

	void RendererVk::AddRenderpass( std::function<void(CommandEncoder&)> func)
	{
		
	}
}