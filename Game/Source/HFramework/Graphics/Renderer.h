#pragma once

#include "../Core/Window.h"
#include "CommandEncoder.h"

namespace hf
{
	class Renderer
	{
	public:

		static Renderer* CreateRenderer(Window* window);

		static void DestroyRenderer(Renderer* renderer);

		virtual bool BeginFrame() = 0;

		virtual void EndFrame() = 0;

		
		virtual void AddRenderpass(std::function<void(CommandEncoder&)> func) = 0;

	protected: 

		virtual void Init(Window* window) = 0;

		virtual void Destroy() = 0;



	private:
	};


}
