#pragma once


#include "../Graphics/Format.h"
#include "VulkanInclude.h"

namespace hf
{
	namespace vulkan
	{
		static VkFormat FormatTable[] = {

			VK_FORMAT_UNDEFINED,

			VK_FORMAT_R8_UNORM,
			VK_FORMAT_R8G8_UNORM,
			VK_FORMAT_R8G8B8_UNORM,
			VK_FORMAT_R8G8B8A8_UNORM,

			VK_FORMAT_R8_SNORM,
			VK_FORMAT_R8G8_SNORM,
			VK_FORMAT_R8G8B8_SNORM,
			VK_FORMAT_R8G8B8A8_SNORM,

			VK_FORMAT_R16_SFLOAT,
			VK_FORMAT_R16G16_SFLOAT,
			VK_FORMAT_R16G16B16_SFLOAT,
			VK_FORMAT_R16G16B16A16_SFLOAT,

			VK_FORMAT_R32_SFLOAT,
			VK_FORMAT_R32G32_SFLOAT,
			VK_FORMAT_R32G32B32_SFLOAT,
			VK_FORMAT_R32G32B32A32_SFLOAT,

			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,

			VK_FORMAT_R8_SRGB,
			VK_FORMAT_R8G8_SRGB,
			VK_FORMAT_R8G8B8_SRGB,
			VK_FORMAT_R8G8B8A8_SRGB,

			VK_FORMAT_B8G8R8A8_SRGB
		};

		static inline Format FromVulkan(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_UNDEFINED:
				return Format::None;
				break;

			case VK_FORMAT_R8_UNORM:
				return Format::R8U;
				break;
			case VK_FORMAT_R8G8_UNORM:
				return Format::RG8U;
				break;
			case VK_FORMAT_R8G8B8_UNORM:
				return Format::RGB8U;
				break;
			case VK_FORMAT_R8G8B8A8_UNORM:
				return Format::RGBA8U;
				break;

			case VK_FORMAT_R8_SNORM:
				return Format::R8S;
				break;
			case VK_FORMAT_R8G8_SNORM:
				return Format::RG8S;
				break;
			case VK_FORMAT_R8G8B8_SNORM:
				return Format::RGB8S;
				break;
			case VK_FORMAT_R8G8B8A8_SNORM:
				return Format::RGBA8S;
				break;

			case VK_FORMAT_R16_SFLOAT:
				return Format::R16F;
				break;
			case VK_FORMAT_R16G16_SFLOAT:
				return Format::RG16F;
				break;
			case VK_FORMAT_R16G16B16_SFLOAT:
				return Format::RGB16F;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return Format::RGBA16F;
				break;

			case VK_FORMAT_R32_SFLOAT:
				return Format::R32F;
				break;
			case VK_FORMAT_R32G32_SFLOAT:
				return Format::RG32F;
				break;
			case VK_FORMAT_R32G32B32_SFLOAT:
				return Format::RGB32F;
				break;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return Format::RGBA32F;
				break;

			case VK_FORMAT_D32_SFLOAT:
				return Format::D32;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return Format::D24_S8;
				break;
			case VK_FORMAT_R8_SRGB:
				return Format::R8_SRGB;
				break;
			case VK_FORMAT_R8G8_SRGB:
				return Format::RG8_SRGB;
				break;
			case VK_FORMAT_R8G8B8_SRGB:
				return Format::RGB8_SRGB;
				break;
			case VK_FORMAT_R8G8B8A8_SRGB:
				return Format::RGBA8_SRGB;
				break;
			case VK_FORMAT_B8G8R8A8_SRGB:
				return Format::BGRA8_SRGB;
				break;
			}
		}
	}
}