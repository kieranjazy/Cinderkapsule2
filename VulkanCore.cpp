#pragma once
#include "VulkanCore.h"
#include "VulkanHelper.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanGraphicsPipeline.h"
#include "vulkan/vulkan.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include <iostream>
#include <memory>
#include <set>



namespace CinderVk {
	struct VulkanCore::impl { //All VulkanCore inner workings go in here
	public:
		VulkanCore* parent;

		SDL_Window* window = nullptr;
		vk::SurfaceKHR surfaceKHR;
		vk::UniqueInstance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue, presentQueue;
		VmaAllocator allocator;
		

		std::unique_ptr<vk::DispatchLoaderDynamic> dldiPtr = nullptr;
		std::unique_ptr<VulkanSwapchain> swapchainPtr = nullptr;
		std::unique_ptr<VulkanRenderpass> renderpassPtr = nullptr;
		std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayoutPtr = nullptr;
		std::unique_ptr<VulkanGraphicsPipeline> graphicsPipelinePtr = nullptr;
		VkDebugUtilsMessengerEXT debugMessenger;

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
		};

		const uint32_t VULKAN_VERSION = VK_API_VERSION_1_1;

		~impl() {
			cleanup();
		}

		impl(VulkanCore* _parent) : parent(_parent) {
			initWindow();
			//initVulkan();
		}

		const void initWindow() {
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
				throw std::runtime_error("Failed to initialize SDL.");
			
			window = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2560, 1440, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		}

		const void initVulkan() {
			createInstance();

			if (enableValidationLayers)
				createDebugMessenger();

			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
			setupVmaAllocator();

			swapchainPtr = std::make_unique<VulkanSwapchain>(parent);
			renderpassPtr = std::make_unique<VulkanRenderpass>(parent);
			descriptorSetLayoutPtr = std::make_unique<VulkanDescriptorSetLayout>(parent);
			graphicsPipelinePtr = std::make_unique<VulkanGraphicsPipeline>(parent);

			//createFramebuffers();
			//createCommandPool();
			//createTextureSampler();

			//loadModels();
			
			//createUniformBuffers();
			//createLightingBuffer();
			//createDescriptorPool();
			//createDescriptorSets();
			//createCommandBuffers();
			//createSyncObjects();

		}

		const void createInstance() {
			if (enableValidationLayers && ![&]() -> bool { //If validation layers are enabled but they are not supported, throw a runtime error.
				auto availableLayers = vk::enumerateInstanceLayerProperties();

				for (const char* layerName : validationLayers) {
					bool layerFound = false;

					for (const auto& layerProperties : availableLayers) {
						if (strcmp(layerName, layerProperties.layerName) == 0) {
							layerFound = true;
							break;
						}
					}

					if (!layerFound)
						return false;
				}

				return true;
			}()) throw std::runtime_error("Validation layers requested but not available.");

			vk::ApplicationInfo applicationInfo("Cinder", 1, "Cinderkapsule", 1, VULKAN_VERSION); //1.1 for VMA atm

			auto extensions = getRequiredExtensions();

			vk::InstanceCreateInfo createInfo(
				{},
				&applicationInfo,
				0, nullptr,
				static_cast<uint32_t>(extensions.size()), extensions.data()
			);

			if (enableValidationLayers) {
				vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(vk::DebugUtilsMessengerCreateFlagsEXT(), 
					vk::DebugUtilsMessageSeverityFlagsEXT(),
					vk::DebugUtilsMessageTypeFlagsEXT(),
					debugCallback, nullptr
				);

				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else {
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}

			try {
				instance = vk::createInstanceUnique(createInfo, nullptr);
				dldiPtr = std::make_unique<vk::DispatchLoaderDynamic>(*instance, vkGetInstanceProcAddr);
			}
			catch (vk::SystemError err) {
				throw std::runtime_error("failed to create instance!");
			}
		}

		const void createDebugMessenger() {
			if (!enableValidationLayers) return; 

			const vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo({}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
				debugCallback, nullptr
			);

			debugMessenger = instance->createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, *dldiPtr);
		}

		void createSurface() {
			SDL_bool r = SDL_Vulkan_CreateSurface(window, VkInstance(instance.get()), reinterpret_cast<VkSurfaceKHR*>(&surfaceKHR));
			if (r != SDL_TRUE)
				throw std::runtime_error("Failed to create Vulkan surface on the SDL window.");
		}

		const void createLogicalDevice() {
			Helper::QueueFamilyIndices indices = Helper::findQueueFamilies(physicalDevice, surfaceKHR);

			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				vk::DeviceQueueCreateInfo queueCreateInfo({}, queueFamily, 1, &queuePriority);
				queueCreateInfos.push_back(queueCreateInfo);
			}

			vk::PhysicalDeviceFeatures deviceFeatures{};
			deviceFeatures.samplerAnisotropy = VK_TRUE;
			deviceFeatures.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;

			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
			indexingFeatures.pNext = nullptr;
			indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
			indexingFeatures.runtimeDescriptorArray = VK_TRUE;

			vk::DeviceCreateInfo createInfo{};
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			createInfo.pNext = &indexingFeatures;

			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			} else createInfo.enabledLayerCount = 0;

			device = physicalDevice.createDevice(createInfo);

			if (device == VK_NULL_HANDLE) //Might need to be nullptr
				throw std::runtime_error("Failed to create logical device.");

			device.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
			device.getQueue(indices.presentFamily.value(), 0, &presentQueue);
		}

		const void setupVmaAllocator() {
			VmaAllocatorCreateInfo allocatorInfo{};
			allocatorInfo.vulkanApiVersion = VULKAN_VERSION;
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = device;
			allocatorInfo.instance = *instance;
			allocatorInfo.flags = {};

			if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
				throw std::runtime_error("VMA allocator could not be created.");
		}

		const void drawFrame() {

		}

		const void cleanup() {
			//device.waitIdle();

			swapchainPtr.reset();
			device.destroy(nullptr);
			
			if (enableValidationLayers)
				instance->destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, *dldiPtr);

			
			instance->destroySurfaceKHR(surfaceKHR, nullptr);
			
			SDL_DestroyWindow(window);
			SDL_Quit();
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}

		void pickPhysicalDevice() {
			auto physicalDevices = instance->enumeratePhysicalDevices();

			if (physicalDevices.empty()) throw std::runtime_error("Failed to find GPUs with Vulkan support.");

			for (const auto& device : physicalDevices) {
				if (isDeviceSuitable(device)) {
					physicalDevice = device;
					break;
				}
			}

			if (physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a suitable GPU.");
		}

		bool isDeviceSuitable(vk::PhysicalDevice device) {
			Helper::QueueFamilyIndices indices = Helper::findQueueFamilies(device, surfaceKHR);

			bool swapchainAdequate = false;
			bool extensionsSupported = checkDeviceExtensionSupport(device);
			
			if (extensionsSupported) {
				Helper::SwapchainSupportDetails swapchainSupport = Helper::querySwapchainSupport(device, surfaceKHR);
				swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures = device.getFeatures();
			
			return indices.isComplete() && extensionsSupported && swapchainAdequate && supportedFeatures.samplerAnisotropy;
		}

		bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
			auto availableExtensions = device.enumerateDeviceExtensionProperties();

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const auto& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		}

		std::vector<const char*> getRequiredExtensions() { //Get the required extensions.
			uint32_t sdlExtensionCount = 0;
			const char** sdlExtensions = NULL;
			SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensions);
			sdlExtensions = new const char* [sdlExtensionCount];

			if (!SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensions))
				throw std::runtime_error("Failed to get SDL instance extensions for Vulkan.");

			std::vector<const char*> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);
			delete[] sdlExtensions;

			if (enableValidationLayers)
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			return extensions;
		}

		//void framebufferResizedSwitch();
	};

	VulkanCore::VulkanCore(void) : pImpl(std::make_unique<impl>(this)) {}
	VulkanCore::~VulkanCore() {  }
}

namespace CinderVk {
	const void VulkanCore::tick() {
		//drawFrame();
	}

	const void framebufferResizedProc() {
		//framebufferResizedSwitch()
	}

	vk::PhysicalDevice* VulkanCore::getPhysicalDevicePtr() const {
		return &(pImpl->physicalDevice); //this returns nonsense
	}

	vk::Device* VulkanCore::getLogicalDevicePtr() const {
		return &(pImpl->device); //same
	}

	vk::SurfaceKHR* VulkanCore::getSurfacePtr() const {
		return &(pImpl->surfaceKHR); //make these members public or something?
	}

	SDL_Window** VulkanCore::getWindowPtrPtr() const {
		return &(pImpl->window); //technically pImpl doesn't exist yet because we are calling this function inside the pImpl constructor function
	}

	vk::Format VulkanCore::getSwapchainImageFormat() const {
		return pImpl->swapchainPtr->getSwapchainImageFormat();
	}

	uint32_t VulkanCore::getSwapchainExtentWidth() const {
		return pImpl->swapchainPtr->getSwapchainExtentWidth();
	}

	uint32_t VulkanCore::getSwapchainExtentHeight() const {
		return pImpl->swapchainPtr->getSwapchainExtentHeight();
	}

	vk::Extent2D VulkanCore::getSwapchainExtent() const {
		return pImpl->swapchainPtr->getSwapchainExtent();
	}

	vk::DescriptorSetLayout VulkanCore::getDescriptorSetLayout() const {
		return pImpl->descriptorSetLayoutPtr->getDescriptorSetLayout();
	}

	vk::RenderPass VulkanCore::getRenderPass() const {
		return pImpl->renderpassPtr->getRenderPass();
	}

	void VulkanCore::initVulkan() {
		pImpl->initVulkan();
	}
}

