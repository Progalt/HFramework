#pragma once

#include "../Core/Window.h"
#include "CommandEncoder.h"

namespace hf
{
	class Renderer
	{
	public:

		static Renderer* CreateRenderer();

		static void DestroyRenderer(Renderer* renderer);

		virtual void RegisterWindow(Window* window) = 0;

		virtual bool BeginFrame(Window* window) = 0;

		virtual void EndFrame(Window* window) = 0;

		
		virtual void AddRenderpass(std::function<void(CommandEncoder&)> func) = 0;

	protected: 

		virtual void Init() = 0;

		virtual void Destroy() = 0;



	private:
	};


}
