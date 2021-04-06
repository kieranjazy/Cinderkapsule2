#pragma once
#include "VulkanCore.h"
#include "VulkanHelper.h"
#include "vulkan/vulkan.hpp"
#include <array>

namespace CinderVk {
	class VulkanRenderpass {
	public:
		VulkanRenderpass(VulkanCore* coreRef) : corePtr(coreRef) {
			init();
		}

		~VulkanRenderpass() {
			cleanup();
		}

	private:
		VulkanCore* corePtr;
		vk::RenderPass renderPass;

		void init() { //This class is gonna need a way to access VulkanSwapchain from the VulkanCore ptr.
			vk::AttachmentDescription colourAttachment;
			colourAttachment.format = corePtr->getSwapchainImageFormat();
			colourAttachment.samples = vk::SampleCountFlagBits::e1;
			colourAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			colourAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			colourAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			colourAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			colourAttachment.initialLayout = vk::ImageLayout::eUndefined;
			colourAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

			vk::AttachmentDescription depthAttachment{};
			depthAttachment.format = Helper::findDepthFormat(*corePtr->getPhysicalDevicePtr());
			depthAttachment.samples = vk::SampleCountFlagBits::e1;
			depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
			depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			vk::AttachmentReference colourAttachmentRef{};
			colourAttachmentRef.attachment = 0;
			colourAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

			vk::AttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			vk::SubpassDescription subpass{};
			subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colourAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			std::array<vk::AttachmentDescription, 2> attachments = { colourAttachment, depthAttachment };
			vk::RenderPassCreateInfo renderPassInfo{};
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;

			vk::SubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; //Might need to be changed
			dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (corePtr->getLogicalDevicePtr()->createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) 
				throw std::runtime_error("Failed to create the render pass.");
		}

		void cleanup() {
			corePtr->getLogicalDevicePtr()->destroyRenderPass(renderPass, nullptr);
		}

	};
}
