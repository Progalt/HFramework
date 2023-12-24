#pragma once

#include "../Renderer.h"
#include "../../Vulkan/Device.h"
#include <mutex>
#include "BufferVk.h"

namespace hf
{
	class RendererVk : public Renderer
	{
	public:

		void Init() override;

		void Destroy() override;

		void WaitIdle();


		void RegisterWindow(Window* window) override;

		Format GetSwapchainFormat(Window* window) override; 

		bool BeginFrame(Window* window) override;

		void EndFrame(Window* window) override;

		std::shared_ptr<Buffer> CreateBuffer(const BufferDesc& desc) override;

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

		struct CopyData
		{
			enum class CopyOp
			{
				Buffer, 
				Texture
			};

			CopyOp op;
			size_t stagingOffset;
			size_t size;

			union
			{
				vulkan::Buffer* buffer;
				vulkan::Texture* texture;
			};
		};

		void QueueBufferCopy(void* data, size_t size, vulkan::Buffer* dst)
		{
			CopyData copyData{};
			copyData.op = CopyData::CopyOp::Buffer;
			copyData.size = size;
			copyData.stagingOffset = m_StagingBuffer.offset;
			copyData.buffer = dst;

			if (m_StagingBuffer.offset + size > m_StagingBuffer.size)
			{
				Log::Fatal("Staging Buffer run out of memory");
				return;
			}

			// We then copy the data to the staging buffer
			char* mem = (char*)m_StagingBuffer.m_Mapped;
			mem += m_StagingBuffer.offset;

			memcpy(mem, data, size);

			m_StagingBuffer.offset += size;

			m_CopyData.push(copyData);

			m_StagingBuffer.dataUploaded = true;
			
		}

		void QueueTextureCopy(void* data)
		{

		}

		std::queue<CopyData> m_CopyData;

		struct
		{

			const size_t size = 1 * 1024 * 1024 * 1024;		// 1 MB Staging Buffer
			vulkan::Buffer buffer;
			void* m_Mapped;
			size_t offset;
			vulkan::Semaphore semaphore;
			bool dataUploaded = false;

		} m_StagingBuffer;


		std::mutex m_Mutex;
	};
}