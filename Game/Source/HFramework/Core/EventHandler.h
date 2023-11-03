#pragma once
#include "Window.h"
#include <unordered_map>
#include "KeyCodes.h"

namespace hf
{

	struct ButtonState
	{
		bool held;			// This is true while the key is down. Which could be for 1 or 20 frames or more. This would be used for movement input or similar
		bool pressed;		// this triggers when the key is initially pressed and is only true for that frame. typically use this for gui based stuff or single input events
	};

	class Keyboard
	{
	public:


		static const ButtonState GetKeyState(KeyCode keyCode)
		{
			return m_KeyStates[(int)keyCode];
		}

		static bool IsKeyHeld(KeyCode keyCode)
		{
			return m_KeyStates[(int)keyCode].held;
		}

		static bool WasKeyPressed(KeyCode keyCode)
		{
			return m_KeyStates[(int)keyCode].pressed;
		}

	private:

		friend class EventHandler;

		static ButtonState m_KeyStates[(int)KeyCode::__Count];
	};

	class Mouse
	{
	public:

	private:
	};

	class EventHandler
	{
	public:

		void HandleGlobalEvents()
		{
			SDL_Event evnt;

			while (SDL_PollEvent(&evnt))
			{
				if (evnt.type == SDL_WINDOWEVENT)
				{
					uint32_t id = evnt.window.windowID;

					if (m_Windows.find(id) != m_Windows.end())
						m_Windows[id]->HandleWindowEvents(&evnt);
				}
				
				// Here we handle the events
				switch (evnt.type)
				{
				case SDL_KEYDOWN:
					break;
				case SDL_KEYUP:
					break;
				case SDL_MOUSEBUTTONDOWN:
					break;
				case SDL_MOUSEBUTTONUP:
					break;
				case SDL_MOUSEMOTION:
					break;
				case SDL_MOUSEWHEEL:
					break;
				}
			}
		}

		bool AnyOpen()
		{
			// Loop through all windows and see if at least 1 is open 
			for (auto& win : m_Windows)
				if (win.second->IsOpen())
					return true;

			return false;
		}


		void RegisterWindow(Window* window)
		{
			m_Windows[window->GetWindowID()] = window;
		}
	private:

		std::unordered_map<uint32_t, Window*> m_Windows;

	};
}