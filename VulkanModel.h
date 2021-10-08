#pragma once
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "Vertex.h"
#include <memory>

namespace CinderVk {
	struct TextureStruct {
		vk::DeviceMemory textureImageMemory;
		vk::Image texture;
		vk::ImageView textureImageView;
		vk::Sampler textureSampler;
	};

	struct VulkanModelData {
		VulkanModelData(const std::string& modelLocation, vk::Device& logicalDevice, vk::Queue& graphicsQueue, vk::CommandPool& commandPool, vk::PhysicalDevice& physicalDevice) :
			modelFileLocation(modelLocation), logicalDevicePtr(&logicalDevice), graphicsQueuePtr(&graphicsQueue), commandPoolPtr(&commandPool), physicalDevicePtr(&physicalDevice)
		{};

		~VulkanModelData() {
			logicalDevicePtr->destroyBuffer(vertexBuffer, nullptr);
			logicalDevicePtr->freeMemory(vertexBufferMemory, nullptr);

			logicalDevicePtr->destroyBuffer(indexBuffer, nullptr);
			logicalDevicePtr->freeMemory(indexBufferMemory, nullptr);

			for (auto tStruct : textureStructs) {
				logicalDevicePtr->destroySampler(tStruct.textureSampler, nullptr);
				logicalDevicePtr->destroyImage(tStruct.texture, nullptr);
				logicalDevicePtr->destroyImageView(tStruct.textureImageView, nullptr);
				logicalDevicePtr->freeMemory(tStruct.textureImageMemory, nullptr);
			}
		};

		vk::Device* logicalDevicePtr;
		vk::Queue* graphicsQueuePtr;
		vk::CommandPool* commandPoolPtr;
		vk::PhysicalDevice* physicalDevicePtr;

		std::vector<fastObjMaterial> materialLayers;
		std::vector<TextureStruct> textureStructs;
		std::string modelFileLocation;

		vk::Buffer vertexBuffer;
		vk::Buffer indexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::DeviceMemory indexBufferMemory;

		uint32_t modelIndicesSize, modelVerticesSize; //set these in setupBuffers

		//std::vector<Vertex> vertices;

		uint32_t getModelIndicesSize();
		uint32_t getModelVerticesSize();

		vk::ImageView getImageView(size_t idx);
		vk::Sampler getTextureSampler(size_t idx);

		vk::Buffer getVertexBuffer();
		vk::Buffer getIndexBuffer();

		void loadModelData();

		void setupBuffers(std::vector<Vertex>& verts, std::vector<uint32_t>& indices);
		void setupTextures();

		void createVertexBuffer(std::vector<Vertex>& verts);
		void createIndexBuffer(std::vector<uint32_t>& indices);
	};

	class VulkanModel {
	public:
		VulkanModel(VulkanModelData* modelDataPtr) : dataPtr(modelDataPtr) {
			worldTransform = glm::mat4(1.0f);
			localTransform = glm::mat4(0.0f);
		}

		~VulkanModel() {

		}

		glm::vec3 getPosition();
		glm::quat getRotation();
		float getScale();

		void setPosition(glm::vec3 pos);
		void setRotation(glm::quat rot);

		void update();

		void rotate(glm::vec3 rotateVec, float angle);
		void translate(glm::vec3 translateVec);
		void scale(float scaleFactor);

	private:
		glm::mat4 localTransform;
		glm::mat4 worldTransform;

		vk::DescriptorSet descriptorSet;

		std::shared_ptr<VulkanModelData> dataPtr;

		void setupDescriptorSets();

	};

}
