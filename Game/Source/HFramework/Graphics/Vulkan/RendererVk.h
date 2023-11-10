#pragma once

#include "../Renderer.h"
#include "../../Vulkan/Device.h"

namespace hf
{
	class RendererVk : public Renderer
	{
	public:

		void Init(Window* window) override;

		void Destroy() override;

		void WaitIdle();

		bool BeginFrame() override;

		void EndFrame() override;

		void AddRenderpass(std::function<void(CommandEncoder&)> func) override;

		hf::vulkan::CommandList& GetCurrentFrameCmdList();


		hf::vulkan::Device m_Device;
		hf::vulkan::Swapchain m_Swapchain;
		hf::vulkan::Surface m_Surface;


		std::vector<hf::vulkan::CommandList> m_BaseCommandLists;

		std::vector<hf::vulkan::Semaphore> m_WorkFinished;
		std::vector<hf::vulkan::Semaphore> m_ImageAvailable;

		uint32_t m_CurrentFrameIndex = 0;


		struct RenderPass
		{
			std::vector<hf::vulkan::CommandList> cmdLists;
		};

		std::unordered_map<std::string, RenderPass> m_RenderPasses;

	};
}