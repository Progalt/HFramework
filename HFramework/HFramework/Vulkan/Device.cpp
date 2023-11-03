
#include "Device.h"

namespace hf
{
	namespace vulkan
	{
		void Device::Create(const DeviceCreateInfo& deviceInfo)
		{
			m_AssociatedWindow = deviceInfo.window;
			m_Debug = deviceInfo.validationLayers;


			CreateInstance(deviceInfo);

			SelectPhysicalDevice();

			m_Surface = deviceInfo.window->GetVulkanSurface(m_Instance);

			FindQueueFamilies();

			CreateDevice();

			m_FencePool.Initialise(m_Device, 12);
		}

		void Device::Dispose()
		{
			vkDeviceWaitIdle(m_Device);

			m_FencePool.Dispose();

			for (auto& cmdPools : m_CommandPools)
			{
				vkDestroyCommandPool(m_Device, cmdPools.second, nullptr);
			}

			vkDestroyDevice(m_Device, nullptr);

			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

			if (m_Debug)
			{
				vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			}
			
			vkDestroyInstance(m_Instance, nullptr);
		}

		Swapchain Device::CreateSwapchain()
		{
			Swapchain swapchain;
			swapchain.m_AssociatedWindow = m_AssociatedWindow;
			swapchain.m_AssociatedSurface = m_Surface;
			swapchain.m_ParentDevice = m_Device;
			swapchain.m_PresentQueue = m_PresentQueue;

			swapchain.m_SupportDetails = swapchain.querySwapChainSupport(m_PhysicalDevice);

			swapchain.Create(m_Device, m_GraphicsQueueFamily, m_PresentQueueFamily);

			return swapchain;
		}

		std::vector<Semaphore> Device::CreateSemaphores(uint32_t count)
		{
			std::vector<Semaphore> semaphores(count);

			for (uint32_t i = 0; i < count; i++)
			{
				Semaphore& semaphore = semaphores[i];
				semaphore.m_ParentDevice = m_Device;

				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &semaphore.m_Semaphore) != VK_SUCCESS)
				{
					Log::Fatal("Failed to create Semaphore");
				}
			}
			return semaphores;
		}

		std::vector<CommandList> Device::AllocateCommandLists(Queue queue, CommandListType type, uint32_t count)
		{
			// Command pools are created on the fly when needed
			// and then cached

			CommandQueueIdentifier iden{};
			iden.queue = queue;
			iden.threadNum = 0;

			VkCommandPool pool = nullptr;

			if (m_CommandPools.find(iden) != m_CommandPools.end())
			{
				pool = m_CommandPools[iden];
			}
			else
			{
				pool = CreateNewCommandPool(queue);

				m_CommandPools[iden] = pool;
			}

			VkCommandBufferLevel level;
			switch (type)
			{
			case CommandListType::Primary: level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; break;
			case CommandListType::Secondary: level = VK_COMMAND_BUFFER_LEVEL_SECONDARY; break;
			default: level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; break;
			}

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = pool;
			allocInfo.level = level;
			allocInfo.commandBufferCount = 1;

			std::vector<CommandList> cmdLists(count);
			for (uint32_t i = 0; i < count; i++)
			{
				CommandList& cmdList = cmdLists[i];
				cmdList.m_Device = m_Device;

				if (vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdList.m_Buffer) != VK_SUCCESS)
				{
					Log::Fatal("Failed to allocate command lists");
				}
			}

			return cmdLists;
		}

		void Device::ExecuteSingleUsageCommandList(Queue queue, std::function<void(CommandList&)> func)
		{

		}

		void Device::QueueSubmit(Queue queue, std::vector<CommandList*> cmdLists, Semaphore* wait, Semaphore* signal)
		{
			m_FencePool.Reclaim();

			VkQueue submitQueue = m_GraphicsQueue;

			switch (queue)
			{
			case Queue::Graphics:
				submitQueue = m_GraphicsQueue;
				break;
			case Queue::Compute:
				submitQueue = m_ComputeQueue;
				break;
			case Queue::Transfer:
				submitQueue = m_TransferQueue;
				break;
			}

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { wait->m_Semaphore };

			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			std::vector<VkCommandBuffer> buffers(cmdLists.size());

			VkFence signalFence = m_FencePool.GetNewFence();

			int i = 0;
			for (auto& cmd : cmdLists)
			{
				// We need to grab the command list for the current frame

				cmd->m_FinishedExecution = signalFence;

				buffers[i] = cmd->m_Buffer;
				i++;
			}

			submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
			submitInfo.pCommandBuffers = buffers.data();

			VkSemaphore signalSemaphores[] = { signal->m_Semaphore };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			if (vkQueueSubmit(submitQueue, 1, &submitInfo, signalFence) != VK_SUCCESS)
			{
				Log::Error("Failed to submit command lists to graphics queue");
			}

			//m_FencePool.Reclaim();
		}

		VkCommandPool Device::CreateNewCommandPool(Queue queue)
		{
			uint32_t queueFamily = 0;
			const char* queueName = "";

			switch (queue)
			{
			case Queue::Graphics:
				queueFamily = m_GraphicsQueueFamily;
				queueName = "Graphics";
				break;
			case Queue::Transfer:
				queueFamily = m_TransferQueueFamily;
				queueName = "Transfer";
				break;
			case Queue::Compute:
				queueFamily = m_ComputeQueueFamily;
				queueName = "Compute";
				break;
			}

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VkCommandPool commandPool;

			if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			{
				Log::Fatal("Failed to create command pool");
			}

			Log::Info("Created New Command Pool for Queue: %s", queueName);
			return commandPool;
		}

	}
}