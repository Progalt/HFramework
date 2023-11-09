
#include "Renderer.h"
#include "Vulkan/RendererVk.h"


namespace hf
{
	Renderer* Renderer::CreateRenderer(Window* window)
	{
		Renderer* renderer = new RendererVk;
		renderer->Init(window);
		return renderer;
	}

	void Renderer::DestroyRenderer(Renderer* renderer)
	{
		renderer->Destroy();
		delete renderer;
	}
}