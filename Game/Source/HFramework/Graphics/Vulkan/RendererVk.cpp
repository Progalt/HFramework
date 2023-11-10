
#include "RendererVk.h"

namespace hf
{
	void RendererVk::Init(Window* window)
	{
		hf::vulkan::DeviceCreateInfo deviceInfo{};
#ifdef _DEBUG
		deviceInfo.validationLayers = true;
#else 
		deviceInfo.validationLayers = false;
#endif
		deviceInfo.window = window;

		m_Device.Create(deviceInfo);

		m_WindowData[window];
		WindowData& windowData = m_WindowData[window];

		windowData.surface = m_Device.CreateSurface(window);

		windowData.swapchain = m_Device.CreateSwapchain(&windowData.surface);

		windowData.baseCommandLists = m_Device.AllocateCommandLists(hf::vulkan::Queue::Graphics, hf::vulkan::CommandListType::Primary, windowData.swapchain.GetImageCount());
		windowData.imageAvailable = m_Device.CreateSemaphores(windowData.swapchain.GetImageCount());
		windowData.workFinished = m_Device.CreateSemaphores(windowData.swapchain.GetImageCount());
	}

	void RendererVk::Destroy()
	{



		
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

	bool RendererVk::BeginFrame(Window* window)
	{
		WindowData& windowData = m_WindowData[window];

		windowData.currentFrameIndex = windowData.swapchain.GetCurrentImageIndex();

		if (!windowData.swapchain.AquireNextFrame(&windowData.imageAvailable[windowData.currentFrameIndex]))
			return false;

		return true;

	}

	void RendererVk::EndFrame(Window* window)
	{
		WindowData& windowData = m_WindowData[window];

		m_Device.QueueSubmit(hf::vulkan::Queue::Graphics, { &GetCurrentFrameCmdList(window) }, &windowData.imageAvailable[windowData.currentFrameIndex], &windowData.workFinished[windowData.currentFrameIndex]);

		windowData.swapchain.Present(&windowData.workFinished[windowData.currentFrameIndex]);
	}

	void RendererVk::WaitIdle()
	{
		m_Device.WaitIdle();
	}

	void RendererVk::AddRenderpass( std::function<void(CommandEncoder&)> func)
	{
		
	}
}