#pragma once

#include "Platform.h"
#include <string>
#include <functional>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include "Log.h"
#include "Rect.h"

namespace hf
{
	namespace internal
	{
		struct HitTestData
		{
			IntRect dragZone;
		};

		SDL_HitTestResult WindowHitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data);
		SDL_HitTestResult WindowHitTestCallbackResizable(SDL_Window* win, const SDL_Point* pt, void* data);
	}

	enum class WindowFlags
	{
		Resizable = 1 << 1,
		NoTitleBar = 1 << 2,
		OpenGL = 1 << 3,
		Vulkan = 1 << 4,
		Transparent = 1 << 5
	};

	inline WindowFlags operator|(WindowFlags lh, WindowFlags rh)
	{
		return static_cast<WindowFlags>(
			static_cast<int>(lh) |
			static_cast<int>(rh)
			);
	}

	inline bool operator&(WindowFlags lh, WindowFlags rh)
	{
		return static_cast<int>(lh) &
			static_cast<int>(rh);
	}

	class Window
	{
	public:

		void Create(const std::string& title, const uint32_t initialWidth, const uint32_t intialHeight, WindowFlags flags);

		/* Custom window functions */

		void UseWindowHitTest(bool use)
		{
			m_UseWindowHitTest = use;
			if (m_Resizable)
				SDL_SetWindowHitTest(m_Window, internal::WindowHitTestCallbackResizable, &m_HitTestData);
			else 
				SDL_SetWindowHitTest(m_Window, internal::WindowHitTestCallback, &m_HitTestData);
		}

		void SetDraggableRegion(IntRect area)
		{
			m_HitTestData.dragZone = area;
		}

		void Close();

		void SetAsCurrent();

		void HandleWindowEvents(SDL_Event* ev);

		bool IsOpen() const { return m_IsOpen; }

		void Swap();

		// Attribute changing 

		void SetTitle(const std::string& title);

		uint32_t GetWidth() { return m_Width; }
		uint32_t GetHeight() { return m_Height; }

		SDL_Window* GetWindow();

		void SetEventCallback(std::function<void(SDL_Event*)> evntCallback) { m_EventCallback = evntCallback; }

		bool IsVisible() const { return m_Visible; }

		void SetVisible(bool vis)
		{
			switch (vis)
			{
			case true:
				SDL_ShowWindow(m_Window);
				m_Visible = true;
				break;
			case false:
				SDL_HideWindow(m_Window);
				m_Visible = false;
				break;
			}

		}

		uint32_t GetWindowID() { return m_WindowID; }


		VkSurfaceKHR GetVulkanSurface(VkInstance instance);

		std::vector<const char*> GetInstanceExtensions();

		// Set callbacks

		void SetResizeCallback(std::function<void(uint32_t, uint32_t)> func)
		{
			m_OnResizeCallback = func;
		}

#ifdef HF_PLATFORM_WINDOWS
		HWND GetHWND();
#endif

	private:

		std::function<void(SDL_Event*)> m_EventCallback;

		std::function<void(uint32_t, uint32_t)> m_OnResizeCallback;

		SDL_Window* m_Window;
		SDL_GLContext m_Context;

		bool m_Visible = true;
		bool m_Resizable = false;
		bool m_Minimised = false;

		uint32_t m_WindowID = 0;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		bool m_IsOpen;

		bool m_UseWindowHitTest = false;
		internal::HitTestData m_HitTestData; 
	};
}