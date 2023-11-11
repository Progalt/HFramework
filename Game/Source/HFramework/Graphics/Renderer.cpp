
#include "Renderer.h"
#include "Vulkan/RendererVk.h"


namespace hf
{
	Renderer* Renderer::CreateRenderer()
	{
		Renderer* renderer = new RendererVk;
		renderer->Init();
		return renderer;
	}

	void Renderer::DestroyRenderer(Renderer* renderer)
	{
		renderer->Destroy();
		delete renderer;
	}
}