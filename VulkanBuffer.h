#pragma once
#include "vulkan/vulkan.hpp"

namespace CinderVk {
	void vulkanMapMemory(vk::Device& device, vk::DeviceMemory memory, vk::DeviceSize offset, vk::DeviceSize size, vk::MemoryMapFlags flags, void** pdata);

	void vulkanUnmapMemory(vk::Device& device, vk::DeviceMemory memory);

	void vulkanDestroyBuffer(vk::Device& device, vk::Buffer buffer);

	void vulkanFreeMemory(vk::Device& device, vk::DeviceMemory bufferMemory);

	vk::CommandBuffer beginSingleTimeCommands(vk::CommandPool& commandPool, vk::Device& device);

	void endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::Queue& graphicsQueue, vk::Device& device, vk::CommandPool& commandPool);

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice& physicalDevice);

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size, vk::CommandPool& commandPool, vk::Device& device, vk::Queue& graphicsQueue);

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory, vk::Device& device, vk::PhysicalDevice physicalDevice);
}


