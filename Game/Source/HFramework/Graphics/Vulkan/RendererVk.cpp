
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

		m_Surface = m_Device.CreateSurface(window);

		m_Swapchain = m_Device.CreateSwapchain(&m_Surface);

		m_BaseCommandLists = m_Device.AllocateCommandLists(hf::vulkan::Queue::Graphics, hf::vulkan::CommandListType::Primary, m_Swapchain.GetImageCount());
		m_WorkFinished = m_Device.CreateSemaphores(m_Swapchain.GetImageCount());
		m_ImageAvailable = m_Device.CreateSemaphores(m_Swapchain.GetImageCount());
	}

	void RendererVk::Destroy()
	{


		for (auto& semaphore : m_ImageAvailable)
			semaphore.Dispose();

		for (auto& semaphore : m_WorkFinished)
			semaphore.Dispose();

		

		m_Swapchain.Dispose();
		m_Surface.Dispose();
		m_Device.Dispose();
	}

	hf::vulkan::CommandList& RendererVk::GetCurrentFrameCmdList()
	{
		return m_BaseCommandLists[m_CurrentFrameIndex];
	}

	bool RendererVk::BeginFrame()
	{
		m_CurrentFrameIndex = m_Swapchain.GetCurrentImageIndex();

		if (!m_Swapchain.AquireNextFrame(&m_ImageAvailable[m_CurrentFrameIndex]))
			return false;

		return true;

	}

	void RendererVk::EndFrame()
	{
		m_Device.QueueSubmit(hf::vulkan::Queue::Graphics, { &GetCurrentFrameCmdList() }, &m_ImageAvailable[m_CurrentFrameIndex], &m_WorkFinished[m_CurrentFrameIndex]);

		m_Swapchain.Present(&m_WorkFinished[m_CurrentFrameIndex]);
	}

	void RendererVk::WaitIdle()
	{
		m_Device.WaitIdle();
	}

	void RendererVk::AddRenderpass( std::function<void(CommandEncoder&)> func)
	{
		
	}
}