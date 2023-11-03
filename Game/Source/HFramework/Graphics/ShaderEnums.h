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
}