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
			void AquireNextFrame(Semaphore* imageAvailable);

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

			Texture* GetSwapchainImage(const uint32_t imageIdx) 
			{ 
				uint32_t idx = imageIdx;
				if (idx > m_Images.size())
					idx = 0;

				return &m_Images[imageIdx];
			}

			Format GetImageFormat()
			{
				return FromVulkan(m_Images[0].m_Format);
			}

		private:

			friend class Device;

			Window* m_AssociatedWindow;
			VkSurfaceKHR m_AssociatedSurface;
			VkDevice m_ParentDevice;
			VkQueue m_PresentQueue;

			VkSwapchainKHR m_SwapChain;

		
			std::vector<Texture> m_Images;

			uint32_t m_SwapchainImageIndex = 0;
			uint32_t m_CurrentFrame = 0;

			uint32_t m_GraphicsQueueFamily = 0;
			uint32_t m_PresentQueueFamily = 0;


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