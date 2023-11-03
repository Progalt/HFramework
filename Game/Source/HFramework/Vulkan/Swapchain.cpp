
#include "Swapchain.h"
#include <algorithm>
#include <set>

namespace hf
{
	namespace vulkan
	{
		void Swapchain::Dispose()
		{
			for (auto image : m_Images) 
			{
				vkDestroyImageView(m_ParentDevice, image.m_ImageView, nullptr);
			}

			vkDestroySwapchainKHR(m_ParentDevice, m_SwapChain, nullptr);
		}



		bool Swapchain::AquireNextFrame(Semaphore* imageAvailable)
		{
			if (m_AssociatedWindow->GetWidth() == 0 || m_AssociatedWindow->GetHeight() == 0)
			{
				m_RenderSafe = false;
				return false;
			}

			if (m_AssociatedWindow->GetWidth() != m_CachedWidth || m_AssociatedWindow->GetHeight() != m_CachedHeight)
			{
				Log::Warn("Swapchain size does not match window size: %d != %d or %d != %d", m_AssociatedWindow->GetWidth(), m_CachedWidth, m_AssociatedWindow->GetHeight(), m_CachedHeight);

				vkDeviceWaitIdle(m_ParentDevice);

				Resize();

				vkDeviceWaitIdle(m_ParentDevice);

			}

			if (m_Settings != m_CachedSettings)
			{
				Log::Warn("Swapchain settings don't match");

				vkDeviceWaitIdle(m_ParentDevice);

				Resize();

				vkDeviceWaitIdle(m_ParentDevice);

			}

			uint32_t imageIndex;

			VkResult result = vkAcquireNextImageKHR(m_ParentDevice, m_SwapChain, UINT64_MAX, imageAvailable->m_Semaphore, VK_NULL_HANDLE, &imageIndex);

			m_SwapchainImageIndex = imageIndex;

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				m_RenderSafe = false;
				Log::Warn("Swapchain out of date");

				vkDeviceWaitIdle(m_ParentDevice);

				Resize();

				vkDeviceWaitIdle(m_ParentDevice);


			}
			else
				m_RenderSafe = true;
	

			return true;
		}

		void Swapchain::Present(Semaphore* wait)
		{ 
			if (m_SwapchainImageIndex > m_Images.size() || !m_RenderSafe)
			{
				Log::Warn("Cannot Present as swapchain image index is out of range");
				return;
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &wait->m_Semaphore;

			VkSwapchainKHR swapChains[] = { m_SwapChain };

			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &m_SwapchainImageIndex;
			presentInfo.pResults = nullptr;

			VkResult presentResult = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

			m_CurrentFrame = (m_CurrentFrame + 1) % MaxImagesInFlight;
			m_Resized = false;
		}

		void Swapchain::Resize()
		{
			Dispose();
			Create(m_ParentDevice, m_GraphicsQueueFamily, m_PresentQueueFamily);
			m_Resized = true;
		}

		Swapchain::SwapchainSupportDetails Swapchain::querySwapChainSupport(VkPhysicalDevice device)
		{
			SwapchainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_AssociatedSurface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_AssociatedSurface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_AssociatedSurface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_AssociatedSurface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_AssociatedSurface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats) 
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
				{
					return availableFormat;
				}
			}

			return availableFormats[0];
		
		}

		VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes) 
			{
				if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR && m_Settings.vsync)
				{
					return availablePresentMode;
				}

				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR && !m_Settings.vsync)
				{
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
			{
				return capabilities.currentExtent;
			}
			else
			{
				

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(m_AssociatedWindow->GetWidth()),
					static_cast<uint32_t>(m_AssociatedWindow->GetHeight())
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		void Swapchain::Create(VkDevice device, uint32_t gfxQueueFamily, uint32_t presentQueueFamily)
		{
			m_SupportDetails = querySwapChainSupport(m_PhysicalDevice);
			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_SupportDetails.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(m_SupportDetails.presentModes);
			VkExtent2D extent = chooseSwapExtent(m_SupportDetails.capabilities);

			uint32_t imageCount = m_SupportDetails.capabilities.minImageCount + 1;

			if (m_SupportDetails.capabilities.maxImageCount > 0 && imageCount > m_SupportDetails.capabilities.maxImageCount) 
			{
				imageCount = m_SupportDetails.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = m_AssociatedSurface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			
			if (gfxQueueFamily != presentQueueFamily) 
			{
				uint32_t queueFamilyIndices[2] = { gfxQueueFamily, presentQueueFamily };
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0; // Optional
				createInfo.pQueueFamilyIndices = nullptr; // Optional
			}

			createInfo.preTransform = m_SupportDetails.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) 
			{
				Log::Fatal("Failed to create swapchain");
				return;
			}

			Log::Info("Successfully Created swapchain");

			// Retrieve the images

			vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
			std::vector<VkImage> images(imageCount);
			vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, images.data());

			m_Images.resize(imageCount);

			for (size_t i = 0; i < m_Images.size(); i++)
			{
				m_Images[i].m_Image = images[i];
				m_Images[i].m_Format = surfaceFormat.format;
				m_Images[i].m_InternallyManaged = true;
				m_Images[i].m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
				m_Images[i].m_Width = extent.width;
				m_Images[i].m_Height = extent.height;
				m_Images[i].m_SwapchainImage = true;

				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = m_Images[i].m_Image;
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = m_Images[i].m_Format;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(m_ParentDevice, &createInfo, nullptr, &m_Images[i].m_ImageView) != VK_SUCCESS)
				{
					Log::Fatal("Failed to create swapchain image views");
				}
			}

			Log::Info("Created Swapchain Image views");


			m_GraphicsQueueFamily = gfxQueueFamily;
			m_PresentQueueFamily = presentQueueFamily;
			m_CachedWidth = extent.width;
			m_CachedHeight = extent.height;
			m_CachedSettings = m_Settings;
		}
	}
}