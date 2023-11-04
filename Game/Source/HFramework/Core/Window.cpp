
#include "Window.h"
#include "Log.h"

namespace hf
{

	namespace internal
	{
		SDL_HitTestResult WindowHitTestCallback(SDL_Window* win, const SDL_Point* pt, void* data)
		{
			HitTestData* hitData = (HitTestData*)data;

			if (hitData->dragZone.Contains(pt->x, pt->y))
			{
				return SDL_HITTEST_DRAGGABLE;
			}

			return SDL_HITTEST_NORMAL;
		}

		SDL_HitTestResult WindowHitTestCallbackResizable(SDL_Window* win, const SDL_Point* pt, void* data)
		{
			HitTestData* hitData = (HitTestData*)data;

			const int MOUSE_GRAB_PADDING = 10;

			if (hitData->dragZone.Contains(pt->x, pt->y))
			{
				return SDL_HITTEST_DRAGGABLE;
			}

			int Width, Height;
			SDL_GetWindowSize(win, &Width, &Height);

			if (pt->y < MOUSE_GRAB_PADDING)
			{
				if (pt->x < MOUSE_GRAB_PADDING)
				{
					return SDL_HITTEST_RESIZE_TOPLEFT;
				}
				else if (pt->x > Width - MOUSE_GRAB_PADDING)
				{
					return SDL_HITTEST_RESIZE_TOPRIGHT;
				}
				else
				{
					return SDL_HITTEST_RESIZE_TOP;
				}
			}
			else if (pt->y > Height - MOUSE_GRAB_PADDING)
			{
				if (pt->x < MOUSE_GRAB_PADDING)
				{
					return SDL_HITTEST_RESIZE_BOTTOMLEFT;
				}
				else if (pt->x > Width - MOUSE_GRAB_PADDING)
				{
					return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
				}
				else
				{
					return SDL_HITTEST_RESIZE_BOTTOM;
				}
			}
			else if (pt->x < MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_LEFT;
			}
			else if (pt->x > Width - MOUSE_GRAB_PADDING)
			{
				return SDL_HITTEST_RESIZE_RIGHT;
			}
			

			return SDL_HITTEST_NORMAL;
		}

#ifdef HF_PLATFORM_WINDOWS

		WNDPROC SDLWindowProc;

		LRESULT CALLBACK SDLOverrideWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static RECT border_thickness;
			int BORDERWIDTH = 1;

			switch (uMsg)
			{
				/*case WM_CREATE:
				{
					SetRectEmpty(&border_thickness);
					if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_THICKFRAME)
					{
						AdjustWindowRectEx(&border_thickness, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_CAPTION, FALSE, NULL);
						border_thickness.left *= -1;
						border_thickness.top *= -1;
					}
					else if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_BORDER)
					{
						SetRect(&border_thickness, 1, 1, 1, 1);
					}

					MARGINS margins = { 0 };
					DwmExtendFrameIntoClientArea(hwnd, &margins);
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
					break;
				}*/
			case WM_NCCALCSIZE:
			{
				if (wParam)
				{
					//NCCALCSIZE_PARAMS* Params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
					//Params->rgrc[0].top += BORDERWIDTH; // rgrc[0] is what makes this work, don't know what others (rgrc[1], rgrc[2]) do, but why not change them all?
					//Params->rgrc[0].left += BORDERWIDTH;
					//Params->rgrc[0].bottom += BORDERWIDTH;
					//Params->rgrc[0].right += BORDERWIDTH;
					return 0;
				}
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
			break;
			}

			return SDLWindowProc(hwnd, uMsg, wParam, lParam);
		}

#endif
	}

	void Window::Create(const std::string& title, const uint32_t initialWidth, const uint32_t intialHeight, WindowFlags flags)
	{


		int winFlags = SDL_WINDOW_SHOWN;

		if (flags & WindowFlags::OpenGL)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

			winFlags |= SDL_WINDOW_OPENGL;
		}

		if (flags & WindowFlags::Resizable)
			m_Resizable = true;

		if (flags & WindowFlags::Resizable && !(flags & WindowFlags::NoTitleBar))
			winFlags |= SDL_WINDOW_RESIZABLE;

		if (flags & WindowFlags::Vulkan)
			winFlags |= SDL_WINDOW_VULKAN;

		if (flags & WindowFlags::NoTitleBar)
			winFlags |= SDL_WINDOW_BORDERLESS;


		m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, initialWidth, intialHeight, winFlags);

		if (!m_Window)
		{
			// Failed to create window
			Log::Fatal("Failed to create window: %s", SDL_GetError());

			return;
		}

		// Grab the window ID and store it so we can filter events later on
		m_WindowID = SDL_GetWindowID(m_Window);

		if (flags & WindowFlags::OpenGL)
		{
			m_Context = SDL_GL_CreateContext(m_Window);

			if (!m_Context)
			{
				// Failed to create OpenGL Context
				Log::Fatal("Failed to create OpenGL Context: %s", SDL_GetError());
				 
				return;
			}
		}

		// Get the window size
		// Some flags might change the size when it is opened
		int w, h;
		SDL_GetWindowSizeInPixels(m_Window, &w, &h);

		m_Width = static_cast<uint32_t>(w);
		m_Height = static_cast<uint32_t>(h);

		m_IsOpen = true;

		// Set the current context as current
		SetAsCurrent();

		Log::Info("Successfully created window");

#ifdef HF_PLATFORM_WINDOWS

		// TODO: Need to add a method to change the colour of the window border
		// TODO: Window resizing is currently broken 

		// All this makes the window look like a window
		// Its a pain and requires rerouting the SDL wndproc through our own and handling some events ourselves
		if (flags & WindowFlags::NoTitleBar)
		{
			HWND hwnd = GetHWND();

			LONG_PTR lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
			// I reset and set new styles this is so there are no styles that could cause issues
			// To preserve window animations we need the min max box sizes 
			lStyle = 0;		// Reset the style
			lStyle = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION;	// Add new specific styles
			SetWindowLongPtr(hwnd, GWL_STYLE, lStyle);

			// Overriding the window proc by storing the SDL one and then setting our own
			// The SDL one is called from our custom wndproc
			// This is only done for this window Style to make it less of a hassle 
			internal::SDLWindowProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			(WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(internal::SDLOverrideWindowProc));

			// Set our borders
			// This makes it so we get the drop shadow effect around the window
			MARGINS borderless = { 1,1,1,1 };
			DwmExtendFrameIntoClientArea(hwnd, &borderless);
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);


		}
#endif
	}

	void Window::Close()
	{
		if (m_Window && m_IsOpen)
		{
			if (m_Context)
				SDL_GL_DeleteContext(m_Context);

			SDL_DestroyWindow(m_Window);

			m_IsOpen = false;
		}
	}

	void Window::SetAsCurrent()
	{
		if (m_Context)
			SDL_GL_MakeCurrent(m_Window, m_Context);
	}

	void Window::HandleWindowEvents(SDL_Event* ev)
	{

		// Filter out the events for other windows

		if (ev->window.windowID != m_WindowID)
			return;

		if (ev->type == SDL_WINDOWEVENT)
		{



			switch (ev->window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:

				Close();
				break;
			case SDL_WINDOWEVENT_RESIZED:
			{
				m_Width = ev->window.data1;
				m_Height = ev->window.data2;

				if (m_OnResizeCallback)
					m_OnResizeCallback(m_Width, m_Height);

				break;
			}
			case SDL_WINDOWEVENT_EXPOSED:
			{
				// This event is triggered every time the window is exposed again and needs a redraw
				// We want to get the size again if its not minmised so we start redrawing
				if (m_Minimised)
				{
					int x, y;
					SDL_GetWindowSize(m_Window, &x, &y);

					m_Width = x;
					m_Height = y;

					m_Minimised = false;
				}
			}
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				m_Width = 0;
				m_Height = 0;
				m_Minimised = true;
				break;
			}


		}

		if (m_EventCallback)
			m_EventCallback(ev);
	}

	void Window::SetTitle(const std::string& title)
	{
		SDL_SetWindowTitle(m_Window, title.c_str());
	}


	SDL_Window* Window::GetWindow()
	{
		return m_Window;
	}

	void Window::Swap()
	{
		if (m_Context)
			SDL_GL_SwapWindow(m_Window);
	}

	VkSurfaceKHR Window::GetVulkanSurface(VkInstance instance)
	{
		VkSurfaceKHR surface = nullptr;

		bool succeeded = SDL_Vulkan_CreateSurface(m_Window, instance, &surface);

		if (!succeeded)
		{
			// Failed to create surface

			Log::Fatal("Failed to Create Vulkan Surface from window");
		}

		return surface;


	}

	std::vector<const char*> Window::GetInstanceExtensions()
	{
		unsigned int count = 0;

		if (!SDL_Vulkan_GetInstanceExtensions(m_Window, &count, nullptr))
		{
			// Failed to get the count of instance extensions

			Log::Fatal("Failed to get vulkan instance extension count");
		}

		std::vector<const char*> output(count);

		if (!SDL_Vulkan_GetInstanceExtensions(m_Window, &count, &output[0]))
		{
			// Failed to put instance extensions in output vector

			Log::Fatal("Failed to get vulkan instance extensions");
		}

		return output;
	}

#ifdef HF_PLATFORM_WINDOWS
	HWND Window::GetHWND()
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(m_Window, &wmInfo);
		HWND hwnd = wmInfo.info.win.window;
		return hwnd;
	}
#endif

}