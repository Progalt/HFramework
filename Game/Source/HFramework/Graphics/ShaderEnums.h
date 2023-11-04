#pragma once

namespace hf
{
	enum class ShaderStage
	{
		Vertex,
		Fragment,
		Compute
	};

	enum class TopologyMode
	{
		Triangles,
		Lines
	};

	enum class CullMode
	{
		None,
		Front, 
		Back
	};


	enum class FilterMode
	{
		Linear,
		Nearest
	};

	enum class WrapMode
	{
		Repeat, 
		ClampToEdge
	};
}