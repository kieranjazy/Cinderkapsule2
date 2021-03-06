#pragma once
#include "VulkanHelper.h"
#include "VulkanCore.h"
#include "VulkanTexture.h"
#include "SDL2/SDL_vulkan.h"
#include <iostream>

namespace CinderVk {
	class VulkanSwapchain {
	public:
		VulkanSwapchain(VulkanCore* coreRef) : corePtr(coreRef){
			init();
		}

		void createDepthResourcesPublic() {
			createDepthResources();
		}

		void createFramebuffersPublic() {
			createFramebuffers();
		}

		vk::Format getSwapchainImageFormat() {
			return swapchainImageFormat;
		}

		uint32_t getSwapchainExtentWidth() {
			return swapchainExtent.width;
		}

		uint32_t getSwapchainExtentHeight() {
			return swapchainExtent.height;
		}

		vk::Extent2D getSwapchainExtent() {
			return swapchainExtent;
		}


		~VulkanSwapchain() {
			cleanup();
		}

	private:
		vk::SwapchainKHR swapchain;
		vk::Format swapchainImageFormat;
		vk::Extent2D swapchainExtent;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapchainImageViews;
		std::vector<vk::Framebuffer> swapchainFramebuffers;

		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;

		VulkanCore* corePtr;
		SDL_Window** window;

		void init() {
			window = corePtr->getWindowPtrPtr();
			Helper::SwapchainSupportDetails swapchainSupport = Helper::querySwapchainSupport(*corePtr->getPhysicalDevicePtr(), *corePtr->getSurfacePtr());

			vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
			vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
			vk::Extent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

			uint32_t imageCount = swapchainSupport.capabilities.maxImageCount;

			if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
				imageCount = swapchainSupport.capabilities.maxImageCount;

			vk::SwapchainCreateInfoKHR createInfo{};
			createInfo.surface = *corePtr->getSurfacePtr();
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

			Helper::QueueFamilyIndices indices = Helper::findQueueFamilies(*corePtr->getPhysicalDevicePtr(), *corePtr->getSurfacePtr());
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			} else {
				createInfo.imageSharingMode = vk::SharingMode::eExclusive;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}

			createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;


			if (corePtr->getLogicalDevicePtr()->createSwapchainKHR(&createInfo, nullptr, &swapchain) != vk::Result::eSuccess)
				throw std::runtime_error("Failed to create the swapchain.");

			corePtr->getLogicalDevicePtr()->getSwapchainImagesKHR(swapchain, &imageCount, nullptr);
			swapchainImages.resize(imageCount);
			corePtr->getLogicalDevicePtr()->getSwapchainImagesKHR(swapchain, &imageCount, swapchainImages.data());
			
			swapchainImageFormat = surfaceFormat.format;
			swapchainExtent = extent;

			createSwapchainImageViews();
		}

		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
					return availableFormat;

			}

			return availableFormats[0];
		}

		vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == vk::PresentModeKHR::eMailbox)
					return availablePresentMode;

			}

			return vk::PresentModeKHR::eFifo;
		}

		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != UINT32_MAX)
				return capabilities.currentExtent;

			int width, height;
			SDL_Vulkan_GetDrawableSize(*window, &width, &height);

			vk::Extent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			return actualExtent;
		}

		void createSwapchainImageViews() {
			swapchainImageViews.resize(swapchainImages.size());

			for (uint32_t i = 0; i < swapchainImages.size(); i++)
				swapchainImageViews[i] = Helper::createImageView(swapchainImages[i], swapchainImageFormat, vk::ImageAspectFlagBits::eColor, *corePtr->getLogicalDevicePtr());
		}

		const void createDepthResources() {
			vk::Format depthFormat = Helper::findDepthFormat(*corePtr->getPhysicalDevicePtr());

			createImage(getSwapchainExtentWidth(), getSwapchainExtentHeight(),
				depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
				depthImage, depthImageMemory, *corePtr->getLogicalDevicePtr(), *corePtr->getPhysicalDevicePtr()
			);

			depthImageView = Helper::createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, *corePtr->getLogicalDevicePtr());
		}

		void createFramebuffers() {
			swapchainFramebuffers.resize(swapchainImageViews.size());

			for (size_t i = 0; i != swapchainImageViews.size(); i++) {
				std::array<vk::ImageView, 2> attachments = {
					swapchainImageViews[i],
					depthImageView
				};

				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.renderPass = corePtr->getRenderPass();
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = swapchainExtent.width;
				framebufferInfo.height = swapchainExtent.height;
				framebufferInfo.layers = 1;

				if ((*corePtr->getLogicalDevicePtr()).createFramebuffer(&framebufferInfo, nullptr, &swapchainFramebuffers[i]) != vk::Result::eSuccess) {
					throw std::runtime_error("Failed to create the framebuffer.");
				}
			}
		}

		void cleanup() {
			corePtr->getLogicalDevicePtr()->destroyImageView(depthImageView, nullptr);
			corePtr->getLogicalDevicePtr()->destroyImage(depthImage, nullptr);
			corePtr->getLogicalDevicePtr()->freeMemory(depthImageMemory, nullptr);

			for (size_t i = 0; i != swapchainFramebuffers.size(); i++) {
				corePtr->getLogicalDevicePtr()->destroyFramebuffer(swapchainFramebuffers[i], nullptr);
			}


			corePtr->getLogicalDevicePtr()->destroySwapchainKHR(swapchain, nullptr);
		}
	};
}

