#pragma once
#include "../Core/Window.h"
#include "../Graphics/Format.h"
#include "VulkanInclude.h"
#include <set>
#include "Texture.h"
#include "Semaphore.h"
#include "FormatConvert.h"

namespace hf
{
	namespace vulkan
	{
		const uint32_t MaxImagesInFlight = 3;

		class Swapchain
		{
		public:

			void Dispose();

			/// <summary>
			/// Attemps to aquire the next frame for rendering
			/// </summary>
			/// <param name="imageAvailable"> a semaphore that is signalled when the frame is aquired</param>
			bool AquireNextFrame(Semaphore* imageAvailable);

			/// <summary>
			/// Present swapchain to window
			/// </summary>
			/// <param name="wait"> a semaphore to wait on before present</param>
			void Present(Semaphore* wait);

			void Resize();

			uint32_t GetImageCount() const 
			{
				return m_Images.size();
			}

			uint32_t GetCurrentImageIndex() const 
			{
				return m_CurrentFrame;
			}

			Texture* GetSwapchainImage() 
			{ 
				return &m_Images[m_SwapchainImageIndex];
			}

			Format GetImageFormat()
			{
				return FromVulkan(m_Images[0].m_Format);
			}

			bool IsAssociatedWithWindow(Window* win)
			{
				if (win == m_AssociatedWindow)
					return true;

				return false;
			}


		private:

			friend class Device;

			Window* m_AssociatedWindow;
			VkSurfaceKHR m_AssociatedSurface;
			VkDevice m_ParentDevice;
			VkQueue m_PresentQueue;
			VkPhysicalDevice m_PhysicalDevice;

			struct SwapchainSettings
			{
				bool vsync = false;

				bool operator!=(const SwapchainSettings& s2)
				{
					if (s2.vsync != vsync)
						return true;

					return false;
				}
			} m_Settings;

			VkSwapchainKHR m_SwapChain;

			SwapchainSettings m_CachedSettings;
		
			std::vector<Texture> m_Images;

			uint32_t m_SwapchainImageIndex = 0;
			uint32_t m_CurrentFrame = 0;

			uint32_t m_GraphicsQueueFamily = 0;
			uint32_t m_PresentQueueFamily = 0;

			uint32_t m_CachedWidth = 0;
			uint32_t m_CachedHeight = 0;
			bool m_RenderSafe = false;
			bool m_Resized = false;


			struct SwapchainSupportDetails
			{
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> presentModes;
			} m_SupportDetails;

			SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

			void Create(VkDevice device, uint32_t gfxQueueFamily, uint32_t presentQueueFamil);
		};
	}
}