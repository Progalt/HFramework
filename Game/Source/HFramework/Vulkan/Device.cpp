
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

			VmaAllocatorCreateInfo allocatorCreateInfo = {};
			allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
			allocatorCreateInfo.device = m_Device;
			allocatorCreateInfo.instance = m_Instance;

			if (vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator) != VK_SUCCESS)
			{
				Log::Fatal("Failed to create VMA Allocator");
			}
			
			Log::Info("Created VMA Allocator");

			m_SetAllocator.Init(m_Device);
		}

		void Device::Dispose()
		{
			vkDeviceWaitIdle(m_Device);

			m_FencePool.Dispose();
			m_SetAllocator.Dispose();

			vmaDestroyAllocator(m_Allocator);

			for (auto& setLayout : m_DescriptorSetLayouts)
			{
				vkDestroyDescriptorSetLayout(m_Device, setLayout.second, nullptr);
			}

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

		Swapchain Device::CreateSwapchain(bool vsync)
		{
			Swapchain swapchain;
			swapchain.m_AssociatedWindow = m_AssociatedWindow;
			swapchain.m_AssociatedSurface = m_Surface;
			swapchain.m_ParentDevice = m_Device;
			swapchain.m_PresentQueue = m_PresentQueue;
			swapchain.m_PhysicalDevice = m_PhysicalDevice;
			swapchain.m_Settings.vsync = vsync;

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

		GraphicsPipeline Device::RetrieveGraphicsPipeline(GraphicsPipelineDesc& desc)
		{
			GraphicsPipeline pipeline;

			std::vector<VkDescriptorSetLayout> setLayouts(desc.setLayouts.size());

			for (uint32_t i = 0; i < desc.setLayouts.size(); i++)
			{
				setLayouts[i] = GetSetLayout(desc.setLayouts[i]);
			}

			pipeline.Create(m_Device, desc, setLayouts);
			return pipeline;
		}

		Buffer Device::CreateBuffer(const BufferDesc& desc)
		{
			Buffer buf;
			buf.m_AssociatedDevice = m_Device;
			buf.m_AssociatedAllocator = m_Allocator;

			buf.Create(desc);

			return buf;
		}

		DescriptorSet Device::AllocateDescriptorSet(DescriptorSetLayout layout)
		{
			VkDescriptorSetLayout setLayout = GetSetLayout(layout);

			DescriptorSet set = DescriptorSet();
			set.m_Device = m_Device;
			if (!m_SetAllocator.Allocate(&set.m_Set, setLayout))
			{
				Log::Error("Failed to Allocate Descriptor set");
			}

			return set;
		}

		std::vector<CommandList> Device::AllocateCommandLists(Queue queue, CommandListType type, uint32_t count)
		{
			// Command pools are created on the fly when needed
			// and then cached

			CommandQueueIdentifier iden{};
			iden.queue = queue;
			iden.threadNum = 0;

			VkCommandPool pool = GetCommandPool(iden);

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

		void Device::ExecuteSingleUsageCommandList(Queue queue, std::function<void(CommandList&)> func, Semaphore* signal)
		{
			CommandQueueIdentifier iden{};
			iden.queue = queue;
			iden.threadNum = 0;

			VkCommandPool pool = GetCommandPool(iden);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = pool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			CommandList cmdList;
			cmdList.m_Device = m_Device;
			cmdList.m_SingleUse = true;

			if (vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdList.m_Buffer) != VK_SUCCESS)
			{
				Log::Fatal("Failed to allocate command lists");
			}

			cmdList.Begin();
			func(cmdList);
			cmdList.End();

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdList.m_Buffer;

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

			// If we have a signal semaphore we want to use it
			if (signal)
			{
				VkSemaphore signalSemaphores[] = { signal->m_Semaphore };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;
			}

			vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);

			// TODO: Is this the best thing to do? 
			// Wait if we don't have a signal semaphore for the queue to finish execution
			// This is for safety 
			if (!signal)
				vkQueueWaitIdle(submitQueue);
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


		void Device::QueueWait(Queue queue)
		{
			switch (queue)
			{
			case Queue::Graphics:
				vkQueueWaitIdle(m_GraphicsQueue);
				break;
			case Queue::Compute:
				vkQueueWaitIdle(m_ComputeQueue);
				break;
			case Queue::Transfer:
				vkQueueWaitIdle(m_TransferQueue);
				break;
			}
		}

		VkCommandPool Device::GetCommandPool(const CommandQueueIdentifier& iden)
		{
			VkCommandPool pool = nullptr;

			if (m_CommandPools.find(iden) != m_CommandPools.end())
			{
				pool = m_CommandPools[iden];
			}
			else
			{
				pool = CreateNewCommandPool(iden.queue);

				m_CommandPools[iden] = pool;
			}

			return pool;
		}

		VkDescriptorSetLayout Device::GetSetLayout(DescriptorSetLayout& layout)
		{
			size_t hash = layout.Hash();
			if (m_DescriptorSetLayouts.find(hash) != m_DescriptorSetLayouts.end())
				return m_DescriptorSetLayouts[hash];

			VkDescriptorSetLayout setLayout;

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = layout.m_LayoutBindings.size();
			layoutInfo.pBindings = layout.m_LayoutBindings.data();

			if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
			{
				Log::Error("Failed to create Descriptor Set Layout");
			}

			Log::Info("Created New Unique Descriptor Set Layout");

			m_DescriptorSetLayouts[hash] = setLayout;

			return setLayout;
		}

	}
}