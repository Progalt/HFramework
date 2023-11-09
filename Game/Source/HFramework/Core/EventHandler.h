#pragma once
#include "Window.h"
#include <unordered_map>
#include "KeyCodes.h"
#include <queue>
#include <glm/glm.hpp>

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

		static void LockMouse(bool lock)
		{
			SDL_SetRelativeMouseMode((SDL_bool)lock);
		}

		static glm::uvec2 GetPosition()  
		{
			return m_CurrentPosition;
		}

		static glm::ivec2 GetRelativeMotion()
		{
			return m_RelativeMotion;
		}

	private:

		friend class EventHandler;

		static glm::uvec2 m_CurrentPosition;
		static glm::ivec2 m_RelativeMotion;
	};

	class EventHandler
	{
	public:

		void HandleGlobalEvents()
		{
			SDL_Event evnt;

			// Reset pressed keys

			while (!m_PressedLastFrame.empty())
			{
				Keyboard::m_KeyStates[(int)m_PressedLastFrame.front()].pressed = false;
				m_PressedLastFrame.pop();
			}

			Mouse::m_RelativeMotion = { 0, 0 };

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
					Keyboard::m_KeyStates[(int)FromSDL2Scancode(evnt.key.keysym.scancode)].held = true;
					Keyboard::m_KeyStates[(int)FromSDL2Scancode(evnt.key.keysym.scancode)].pressed = true;
					m_PressedLastFrame.push(FromSDL2Scancode(evnt.key.keysym.scancode));
					break;
				case SDL_KEYUP:
					Keyboard::m_KeyStates[(int)FromSDL2Scancode(evnt.key.keysym.scancode)].held = false;
					break;
				case SDL_MOUSEBUTTONDOWN:
					break;
				case SDL_MOUSEBUTTONUP:
					break;
				case SDL_MOUSEMOTION:
				{
					Mouse::m_CurrentPosition.x = evnt.motion.x;
					Mouse::m_CurrentPosition.y = evnt.motion.y;

					Mouse::m_RelativeMotion.x = evnt.motion.xrel;
					Mouse::m_RelativeMotion.y = evnt.motion.yrel;

				
				}
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

		std::queue<KeyCode> m_PressedLastFrame;

		std::unordered_map<uint32_t, Window*> m_Windows;

	};
}