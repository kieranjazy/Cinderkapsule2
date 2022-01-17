#pragma once
#include "VulkanWrapper.h"
#include <vector>

namespace CinderVk {
	class VulkanDescriptorSetLayout : VulkanWrapper{
	public:
		VulkanDescriptorSetLayout(VulkanCore* coreRef) : VulkanWrapper(coreRef) {
			init();
		}

		vk::DescriptorSetLayout getDescriptorSetLayout() {
			return descriptorSetLayout;
		}
		
		~VulkanDescriptorSetLayout() {
			cleanup();
		}

	private:
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		vk::DescriptorSetLayout descriptorSetLayout;

		void init() {
			addLayoutBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex); //Layout for vertices
	
			for (size_t i = 0; i != 5; i++) { //Layouts for PBR maps
				addLayoutBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment); //Maybe change this to work ny changing descriptor count instead
			}

			addLayoutBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);  //Layout for point lights

			vk::DescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
			layoutInfo.pBindings = layoutBindings.data();
			//layoutInfo.flags = 0;


			if (getCorePtr()->getLogicalDevicePtr()->createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to create the descriptor set layout.");
			}
		}

		void cleanup() {
			getCorePtr()->getLogicalDevicePtr()->destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
		}

		void addLayoutBinding(vk::DescriptorType descriptorType, vk::ShaderStageFlags flags, uint32_t descriptorCount = 1, vk::Sampler* immutableSamplers = nullptr) {
			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = layoutBindings.size();
			layoutBinding.descriptorType = descriptorType;
			layoutBinding.descriptorCount = descriptorCount;
			layoutBinding.stageFlags = flags; //vk::ShaderStage::FragmentBit or the like
			layoutBinding.pImmutableSamplers = immutableSamplers;

			layoutBindings.push_back(layoutBinding);
		}
	};
}