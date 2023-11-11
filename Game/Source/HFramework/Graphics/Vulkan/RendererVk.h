#pragma once

#include "../Renderer.h"
#include "../../Vulkan/Device.h"

namespace hf
{
	class RendererVk : public Renderer
	{
	public:

		void Init() override;

		void Destroy() override;

		void WaitIdle();


		void RegisterWindow(Window* window) override;

		bool BeginFrame(Window* window) override;

		void EndFrame(Window* window) override;

		void AddRenderpass(std::function<void(CommandEncoder&)> func) override;

		hf::vulkan::CommandList& GetCurrentFrameCmdList(Window* window);


		hf::vulkan::Device m_Device;

		struct WindowData
		{
			hf::vulkan::Surface surface;
			hf::vulkan::Swapchain swapchain;
			uint32_t currentFrameIndex = 0;

			std::vector<hf::vulkan::Semaphore> workFinished;
			std::vector<hf::vulkan::Semaphore> imageAvailable;

			std::vector<hf::vulkan::CommandList> baseCommandLists;
		};

		std::unordered_map<Window*, WindowData> m_WindowData;

		WindowData& GetWindowData(Window* wnd)
		{
			return m_WindowData[wnd];
		}




		struct RenderPass
		{
			std::vector<hf::vulkan::CommandList> cmdLists;
		};

		std::unordered_map<std::string, RenderPass> m_RenderPasses;

	};
}