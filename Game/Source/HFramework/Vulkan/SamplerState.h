#pragma once
#include "../Core/Util.h"
#include "../Graphics/ShaderEnums.h"

namespace hf
{
	namespace vulkan
	{
		struct SamplerState
		{
			SamplerState()
			{
				min = FilterMode::Nearest;
				mag = FilterMode::Nearest;

				wrapU = WrapMode::Repeat;
				wrapV = WrapMode::Repeat;
				wrapW = WrapMode::Repeat;
			}

			FilterMode min;
			FilterMode mag;

			WrapMode wrapU;
			WrapMode wrapV;
			WrapMode wrapW;

			float maxAnisotropy = 0.0f;


			bool operator==(const SamplerState& rh) const
			{
				return (min == rh.min && mag == rh.mag && wrapU == rh.wrapU && wrapV == rh.wrapV && wrapW == rh.wrapW && maxAnisotropy == rh.maxAnisotropy);
			}
		};

		struct SamplerStateHash
		{
			size_t operator()(const SamplerState& state) const
			{
				size_t hash = 0;
				hash_combine(hash, state.min);
				hash_combine(hash, state.mag);
				hash_combine(hash, state.wrapU);
				hash_combine(hash, state.wrapV);
				hash_combine(hash, state.wrapW);
				hash_combine(hash, state.maxAnisotropy);
				return hash;
			}
		};
	}
}