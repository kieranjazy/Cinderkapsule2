#pragma once
#include "VulkanModel.h"
#include "VulkanBuffer.h"
#include "fast_obj.h"

namespace CinderVk {
	uint32_t VulkanModelData::getModelIndicesSize() {
		return modelIndicesSize;
	}

	uint32_t VulkanModelData::getModelVerticesSize() {
		return modelVerticesSize;
	}

	vk::ImageView VulkanModelData::getImageView(size_t idx) {
		return textureStructs[idx].textureImageView;
	}

	vk::Sampler VulkanModelData::getTextureSampler(size_t idx) {
		return textureStructs[idx].textureSampler;
	}

	vk::Buffer VulkanModelData::getVertexBuffer() {
		return vertexBuffer;
	}

	vk::Buffer VulkanModelData::getIndexBuffer() {
		return indexBuffer;
	}

	void VulkanModelData::loadModelData() {


	}

	void VulkanModelData::setupBuffers(std::vector<Vertex>& verts, std::vector<uint32_t>& indices) {
		modelIndicesSize = static_cast<uint32_t>(indices.size());
		modelVerticesSize = static_cast<uint32_t>(verts.size());

		createVertexBuffer(verts);
		createIndexBuffer(indices);
	}

	void VulkanModelData::createVertexBuffer(std::vector<Vertex>& verts) {
		vk::DeviceSize bufferSize = verts.size() * sizeof(Vertex);

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *logicalDevicePtr, *physicalDevicePtr);
		
		void* data;
		vulkanMapMemory(*logicalDevicePtr, stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits{}, &data);
		memcpy(data, verts.data(), (size_t)bufferSize);
		vulkanUnmapMemory(*logicalDevicePtr, stagingBufferMemory);

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory, *logicalDevicePtr, *physicalDevicePtr);
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize, *commandPoolPtr, *logicalDevicePtr, *graphicsQueuePtr);

		vulkanDestroyBuffer(*logicalDevicePtr, stagingBuffer);
		vulkanFreeMemory(*logicalDevicePtr, stagingBufferMemory);
	}

	void VulkanModelData::createIndexBuffer(std::vector<uint32_t>& indices) {
		vk::DeviceSize bufferSize = indices.size() * sizeof(uint32_t);
		
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *logicalDevicePtr, *physicalDevicePtr);

		void* data;
		vulkanMapMemory(*logicalDevicePtr, stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits{}, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vulkanUnmapMemory(*logicalDevicePtr, stagingBufferMemory);

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory, *logicalDevicePtr, *physicalDevicePtr);
		copyBuffer(stagingBuffer, indexBuffer, bufferSize, *commandPoolPtr, *logicalDevicePtr, *graphicsQueuePtr);

		vulkanDestroyBuffer(*logicalDevicePtr, stagingBuffer);
		vulkanFreeMemory(*logicalDevicePtr, stagingBufferMemory);
	}

	void VulkanModelData::setupTextures() {
		TextureStruct tStruct;
		std::vector<std::string> maps;

		maps = {
			materialLayers[0].map_Kd.path, //difuse
			materialLayers[0].map_bump.path, //normal
			materialLayers[0].map_Ks.path, //roughness
			materialLayers[0].map_Ns.path, //metallic
			materialLayers[0].map_Ka.path //AO
		};

		for (size_t i = 0; i != maps.size(); i++) {
			//tStruct.texture = cre
		}


	}


}