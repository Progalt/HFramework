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

			m_MainWindow.Create(info.title, info.width, info.height, hf::WindowFlags::Vulkan);

			m_EventHandler.RegisterWindow(&m_MainWindow);

			Start();

			while (m_EventHandler.AnyOpen())
			{
				m_EventHandler.HandleGlobalEvents();


				Update(0.0f);
				Render();

				m_MainWindow.Swap();
			}

			Destroy();

			SDL_Quit();
		}

		EventHandler* GetEventHandler() { return &m_EventHandler; }
		Window* GetMainWindow() { return &m_MainWindow; }

	private:

		EventHandler m_EventHandler;
		Window m_MainWindow;

		void MainLoop()
		{

		}
	};
}