#pragma once
#include "vulkan/vulkan.hpp"

namespace CinderVk {
	void vulkanMapMemory(vk::Device& device, vk::DeviceMemory memory, vk::DeviceSize offset, vk::DeviceSize size, vk::MemoryMapFlags flags, void** pdata) {
		device.mapMemory(memory, offset, size, flags, pdata);
	}

	void vulkanUnmapMemory(vk::Device& device, vk::DeviceMemory memory) {
		device.unmapMemory(memory);
	}

	void vulkanDestroyBuffer(vk::Device& device, vk::Buffer buffer) {
		device.destroyBuffer(buffer, nullptr);
	}

	void vulkanFreeMemory(vk::Device& device, vk::DeviceMemory bufferMemory) {
		device.freeMemory(bufferMemory, nullptr);
	}

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size, vk::CommandPool& commandPool, vk::Device& device, vk::Queue& graphicsQueue) {
		vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

		vk::BufferCopy copyRegion{};
		copyRegion.size = size;
		commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
	}

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory, vk::Device& device, vk::PhysicalDevice physicalDevice) {
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		if (device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create buffer");
		}

		vk::MemoryRequirements memRequirements;
		device.getBufferMemoryRequirements(buffer, &memRequirements);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);
	
		if (device.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to allocate buffer memory");
		}

		device.bindBufferMemory(buffer, bufferMemory, 0);
	}

	vk::CommandBuffer beginSingleTimeCommands(vk::CommandPool& commandPool, vk::Device& device) {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		device.allocateCommandBuffers(&allocInfo, &commandBuffer);

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin(&beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::Queue& graphicsQueue, vk::Device& device, vk::CommandPool& commandPool) {
		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		graphicsQueue.submit(1, &submitInfo, nullptr);
		graphicsQueue.waitIdle();

		device.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice& physicalDevice) {
		vk::PhysicalDeviceMemoryProperties memProperties;
		physicalDevice.getMemoryProperties(&memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find a suitable memory type");
	}
}


