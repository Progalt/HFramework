
#include "BufferVk.h"
#include "RendererVk.h"

namespace hf
{
	void BufferVk::Upload(void* data, size_t size, size_t offset)
	{
		// If its CPU visible we can handle the copy here

		char* mem = (char*)m_MappedBuffer;
		mem += offset;

		if (m_CpuVisible)
		{

			memcpy(mem, data, size);

			return;
		}

		// We need to queue a copy operation in the renderer

		m_Renderer->QueueBufferCopy(data, size, &m_Buffer);

	}
}