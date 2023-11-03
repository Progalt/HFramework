#pragma once
#include "../Core/Window.h"
#include "Swapchain.h"
#include "CommandList.h"
#include "Semaphore.h"
#include "../Core/Util.h"
#include "FencePool.h"

namespace hf
{
	namespace vulkan
	{

		enum class Queue
		{
			Graphics,
			Compute,
			Transfer
		};

		enum class CommandListType
		{
			Primary, 
			Secondary
		};


		struct DeviceCreateInfo
		{
			bool validationLayers;
			Window* window;
		};

		class Device
		{
		public:

			void Create(const DeviceCreateInfo& deviceInfo);
			void Dispose();

			Swapchain CreateSwapchain();
			
			std::vector<CommandList> AllocateCommandLists(Queue queue, CommandListType type, uint32_t count);

			std::vector<Semaphore> CreateSemaphores(uint32_t count);

			void ExecuteSingleUsageCommandList(Queue queue, std::function<void(CommandList&)> func);

			void QueueSubmit(Queue queue, std::vector<CommandList*> cmdLists, Semaphore* wait, Semaphore* signal);

			void WaitIdle() { vkDeviceWaitIdle(m_Device); }

		private:

			Window* m_AssociatedWindow;

			VkInstance m_Instance;
			VkDebugUtilsMessengerEXT m_DebugMessenger;

			bool m_Debug = false;

			VkPhysicalDevice m_PhysicalDevice;

			uint32_t m_PresentQueueFamily = 0;
			uint32_t m_GraphicsQueueFamily = 0;
			uint32_t m_ComputeQueueFamily = 0;
			uint32_t m_TransferQueueFamily = 0;

			VkDevice m_Device;

			VkSurfaceKHR m_Surface;

			VkQueue m_GraphicsQueue;
			VkQueue m_PresentQueue;
			VkQueue m_TransferQueue;
			VkQueue m_ComputeQueue;

			FencePool m_FencePool;

			struct CommandQueueIdentifier
			{
				// Each command pool is associated with a thread and queue
				Queue queue;
				uint32_t threadNum = 0;

				bool operator==(const CommandQueueIdentifier& iden) const
				{
					return (queue == iden.queue && threadNum == iden.threadNum);
				}
			};

			struct CommandQueueIdentifierHash
			{
				size_t operator()(const CommandQueueIdentifier& iden) const
				{
					size_t seed = 0;
					hash_combine(seed, iden.queue);
					hash_combine(seed, iden.threadNum);
					return seed;
				}
			};

			std::unordered_map<CommandQueueIdentifier, VkCommandPool, CommandQueueIdentifierHash> m_CommandPools;

			// Creation functions (Unlike other functions in device they are found in DeviceCreation.cpp)

			void CreateInstance(const DeviceCreateInfo& info);

			void SelectPhysicalDevice();

			void FindQueueFamilies();

			void CreateDevice();

			VkCommandPool CreateNewCommandPool(Queue queue);
		};
	}
}