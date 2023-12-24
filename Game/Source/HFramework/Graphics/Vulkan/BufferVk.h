#pragma once

#include "../Buffer.h"
#include "../../Vulkan/Buffer.h"

namespace hf
{
	class RendererVk;

	class BufferVk : public Buffer
	{
	public:

		void Dispose() override
		{
			m_Buffer.Dispose();
		}

		void Upload(void* data, size_t size, size_t offset = 0) override;

		void* m_MappedBuffer;
		bool m_CpuVisible = false;
		vulkan::Buffer m_Buffer;

		RendererVk* m_Renderer;
	};
}
