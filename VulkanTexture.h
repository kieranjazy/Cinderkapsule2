#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vulkan/vulkan.hpp"
#include "VulkanBuffer.h"

namespace CinderVk {
	vk::Image createTextureImage(const std::string& texturePath, vk::Device& logicalDevice, vk::PhysicalDevice& physicalDevice, vk::DeviceMemory& textureImageMemory, vk::CommandPool& commandPool, vk::Queue& graphicsQueue) {
		int texWidth, texHeight, texChannels;
		vk::Image textureImage;
		stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
			throw std::runtime_error("Failed to load the texture image of: " + texturePath);

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, logicalDevice, physicalDevice);
		void* data;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(logicalDevice, stagingBufferMemory);
		
		stbi_image_free(pixels);

		//createImage(texWidth, texHeight, vk::Format)

	}

	void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory, vk::Device& device, vk::PhysicalDevice& physicalDevice) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		if (device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create image");
		}

		vk::MemoryRequirements memRequirements;
		device.getImageMemoryRequirements(image, &memRequirements);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

		if (device.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to allocate image memory");
		}

		device.bindImageMemory(image, imageMemory, 0);
	}

	void transitionImageLayout(vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandPool& commandPool, vk::Device& device, vk::Queue& graphicsQueue) {
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		//barrier.srcAccessMask = vk::AccessFlagBits::;
		//barrier.dstAccessMask = 0;

		vk::PipelineStageFlags sourceStage, destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			//barrier.srcAccessMask = 0;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition.");
		}


		commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

		endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
	}








}
