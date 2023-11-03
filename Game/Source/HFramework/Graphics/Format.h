#pragma once


namespace hf
{
	enum class Format
	{
		None,

		R8U,
		RG8U,
		RGB8U,
		RGBA8U,

		R8S,
		RG8S,
		RGB8S,
		RGBA8S,

		R16F,
		RG16F,
		RGB16F,
		RGBA16F,

		R32F,
		RG32F,
		RGB32F,
		RGBA32F,

		D32,
		D24_S8,

		R8_SRGB,
		RG8_SRGB,
		RGB8_SRGB,
		RGBA8_SRGB,

		BGRA8_SRGB
	};
}