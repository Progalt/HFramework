
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

			const int resizeBorder = 10;
			int w, h;
			SDL_GetWindowSize(win, &w, &h);

			if (hitData->dragZone.Contains(pt->x, pt->y))
			{
				return SDL_HITTEST_DRAGGABLE;
			}

			if (pt->x < resizeBorder && pt->y < resizeBorder) {
				return SDL_HITTEST_RESIZE_TOPLEFT;
			}
			else if (pt->x > resizeBorder && pt->x < w - resizeBorder && pt->y < resizeBorder) {
				return SDL_HITTEST_RESIZE_TOP;
			}
			else if (pt->x > w - resizeBorder && pt->y < resizeBorder) {
				return SDL_HITTEST_RESIZE_TOPRIGHT;
			}
			else if (pt->x > w - resizeBorder && pt->y > resizeBorder && pt->y < h - resizeBorder) {
				return SDL_HITTEST_RESIZE_RIGHT;
			}
			else if (pt->x > w - resizeBorder && pt->y > h - resizeBorder) {
				return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
			}
			else if (pt->x < w - resizeBorder && pt->x > resizeBorder && pt->y > h - resizeBorder) {
				return SDL_HITTEST_RESIZE_BOTTOM;
			}
			else if (pt->x < resizeBorder && pt->y > h - resizeBorder) {
				return SDL_HITTEST_RESIZE_BOTTOMLEFT;
			}
			else if (pt->x < resizeBorder && pt->y < h - resizeBorder && pt->y > resizeBorder) {
				return SDL_HITTEST_RESIZE_LEFT;
			}

			return SDL_HITTEST_NORMAL;
		}

#ifdef HF_PLATFORM_WINDOWS

		WNDPROC SDLWindowProc;

		LRESULT CALLBACK SDLOverrideWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static RECT border_thickness;

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
				return 0;
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
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				m_Width = ev->window.data1;
				m_Height = ev->window.data2;

				if (m_OnResizeCallback)
					m_OnResizeCallback(m_Width, m_Height);

			}
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				m_Width = 0;
				m_Height = 0;
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