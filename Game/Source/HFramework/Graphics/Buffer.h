#pragma once


namespace hf
{
	enum class BufferType
	{
		Vertex,
		Index,
		Uniform
	};

	struct BufferDesc
	{
		BufferType bufferType;
		bool cpuVisible;
		size_t size;
	};

	class Buffer
	{
	public:

		virtual void Dispose() = 0;

		virtual void Upload(void* data, size_t size, size_t offset = 0) = 0;

	private:
	};
}