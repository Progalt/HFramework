
#include "Device.h"
#include <map>
#include <set>
#include <string>

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


namespace hf
{
	namespace vulkan
	{
		// Validation layer support stuff

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		bool checkValidationLayerSupport() {
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					return false;
				}
			}

			return true;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				Log::Warn("[Vulkan Validation Layer]: %s", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				Log::Error("[Vulkan Validation Layer]: %s", pCallbackData->pMessage);
				break;
			default:
				break;
			}

			return VK_FALSE;
		}

		void Device::CreateInstance(const DeviceCreateInfo& info)
		{

			bool enableValidation = true;
			if (info.validationLayers && !checkValidationLayerSupport())
			{
				Log::Warn("Validation Layers Requested but not available so they will not be enabled");
				enableValidation = false;
			}

			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "VEngine";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "VEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			
			std::vector<const char*> requiredExtensions = info.window->GetInstanceExtensions();

			if (enableValidation) 
			{
				requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			createInfo.enabledExtensionCount = requiredExtensions.size();
			createInfo.ppEnabledExtensionNames = requiredExtensions.data();

			if (enableValidation) 
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else {
				createInfo.enabledLayerCount = 0;
			}

			if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
			{
				Log::Fatal("Failed to create Vulkan Instance");
				return;
			}

			Log::Info("Successfully created Vulkan instance");

			// We can create the debug messenger is needed now

			if (enableValidation)
			{
				VkDebugUtilsMessengerCreateInfoEXT createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				createInfo.pfnUserCallback = debugCallback;
				createInfo.pUserData = nullptr;

				if (vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) 
				{
					Log::Fatal("failed to set up vulkan debug messenger");
					return;
				}

				Log::Info("Setup vulkan debug messenger");
			}
		}

		
		int rateDeviceSuitability(VkPhysicalDevice device) 
		{
		
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			int score = 0;

			// For now just pick a discrete GPU 
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
			{
				score += 1000;
			}


			return score;
		}

		void Device::SelectPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

			if (deviceCount == 0)
				Log::Fatal("No GPUs with vulkan support exist");

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

			std::multimap<int, VkPhysicalDevice> candidates;

			for (const auto& device : devices) {
				int score = rateDeviceSuitability(device);
				candidates.insert(std::make_pair(score, device));
			}


			if (candidates.rbegin()->first > 0) 
			{
				m_PhysicalDevice = candidates.rbegin()->second;
				Log::Info("Selected Vulkan Compatible Physical Device");
			}
			else
			{
				Log::Fatal("Failed to find suitable GPU");
			}
		}

		void Device::FindQueueFamilies()
		{
			// Find queue families. Present is not handled here 

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

			if (queueFamilyCount == 0)
			{
				Log::Fatal("No Queue families found");
			}
			else
			{
				Log::Info("Device Queue Family Count: %d", queueFamilyCount); 
			}


			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

			bool foundPresentQueueFamily = false;
			bool foundGraphicsQueueFamily = false;
			bool foundTransferQueueFamily = false;
			bool foundComputeQueueFamily = false;

			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{


				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !foundTransferQueueFamily)
				{

					m_GraphicsQueueFamily = i;
					foundGraphicsQueueFamily = true;
				}

				if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !foundComputeQueueFamily)
				{

					m_ComputeQueueFamily = i;
					foundComputeQueueFamily = true;
				}

				if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !foundTransferQueueFamily)
				{
					m_TransferQueueFamily = i;
					foundTransferQueueFamily = true;
				}
				else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && m_TransferQueueFamily == m_GraphicsQueueFamily)
				{
					// Try to grab a dedicated transfer queue 
					m_TransferQueueFamily = i;
				}


			}

			if (foundGraphicsQueueFamily)
			{

				
			}
			else
			{

				Log::Fatal("Failed to find Queue family that supports graphics");
			}
		}

		bool checkDeviceExtensionSupport(std::vector<const char*> wantedExtensions, VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


			std::set<std::string> requiredExtensions(wantedExtensions.begin(), wantedExtensions.end());

			//std::cout << "Available Device Extensions: \n";
			for (const auto& extension : availableExtensions)
			{
				//std::cout << " - " << extension.extensionName << std::endl;

				requiredExtensions.erase(extension.extensionName);
			}


			return requiredExtensions.empty();
		}

		void Device::CreateDevice()
		{
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies;

			uniqueQueueFamilies = { m_GraphicsQueueFamily, m_PresentQueueFamily, m_ComputeQueueFamily, m_TransferQueueFamily };

			float queuePriority = 1.0f;


			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderFeature{};
			dynamicRenderFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
			dynamicRenderFeature.dynamicRendering = VK_TRUE;

			VkPhysicalDeviceFeatures2 features{};
			features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features.pNext = &dynamicRenderFeature;
			features.features.samplerAnisotropy = VK_TRUE;

			VkDeviceCreateInfo deviceCreateInfo = {};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceCreateInfo.pNext = &features;

			std::vector<const char*> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			if (!checkDeviceExtensionSupport(deviceExtensions, m_PhysicalDevice))
			{
				Log::Fatal("Device Extensions aren't supported");
			}

			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
			{
				Log::Error("Failed to create device");
			}

			vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_Device, m_TransferQueueFamily, 0, &m_TransferQueue);
			vkGetDeviceQueue(m_Device, m_ComputeQueueFamily, 0, &m_ComputeQueue);

			Log::Info("Successfully Created Vulkan Device and retrieved Queues");
		}
	}
}