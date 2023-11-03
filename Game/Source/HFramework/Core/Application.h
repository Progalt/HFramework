#pragma once

#include "EventHandler.h"
#include "../Vulkan/Device.h"

namespace hf
{
	struct StartInfo
	{
		std::string title;
		uint32_t width, height;
	};

	class Application
	{
	public:

		virtual void Start() { }

		virtual void Tick() { }

		virtual void Update(float deltaTime) { }

		virtual void Render() { }

		virtual void Destroy() { }

		void Run(const StartInfo& info)
		{
			
			if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
			{
				Log::Fatal("Failed to init SDL2");
			}

			m_MainWindow.Create(info.title, info.width, info.height, hf::WindowFlags::Vulkan | hf::WindowFlags::NoTitleBar | hf::WindowFlags::Resizable);
			m_MainWindow.SetDraggableRegion({ 0, 0, 50, 30 });
			m_MainWindow.UseWindowHitTest(true);

			m_EventHandler.RegisterWindow(&m_MainWindow);

			Start();

			while (m_EventHandler.AnyOpen())
			{
				MainLoop();
			}

			Destroy();

			SDL_Quit();
		}

		EventHandler* GetEventHandler() { return &m_EventHandler; }
		Window* GetMainWindow() { return &m_MainWindow; }

	private:

		EventHandler m_EventHandler;
		Window m_MainWindow;

		Uint64 NOW = SDL_GetPerformanceCounter();
		Uint64 LAST = 0;
		double deltaTime = 0;

		void MainLoop()
		{
			LAST = NOW;
			NOW = SDL_GetPerformanceCounter();

			deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001;

			m_EventHandler.HandleGlobalEvents();


			Update((float)deltaTime);
			Render();

			m_MainWindow.Swap();
		}
	};
}