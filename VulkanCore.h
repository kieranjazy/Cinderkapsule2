#pragma once
#include <memory>


namespace vk {
	class PhysicalDevice;
	class Device;
	class SurfaceKHR;
	enum class Format;
	class Extent2D;
	class DescriptorSetLayout;
	class RenderPass;
}

struct SDL_Window;


namespace CinderVk {
	class VulkanCore {
	public:
		
		const void tick();
		const void framebufferResizedSwitch(); //Needed for InputCore to interface with VulkanCore

		vk::PhysicalDevice* getPhysicalDevicePtr() const;
		vk::Device* getLogicalDevicePtr() const;
		vk::SurfaceKHR* getSurfacePtr() const;
		SDL_Window** getWindowPtrPtr() const;
		vk::Format getSwapchainImageFormat() const;
		uint32_t getSwapchainExtentWidth() const;
		uint32_t getSwapchainExtentHeight() const;
		vk::Extent2D getSwapchainExtent() const;
		vk::DescriptorSetLayout getDescriptorSetLayout() const;
		vk::RenderPass getRenderPass() const;

		void initVulkan();

		VulkanCore();
		~VulkanCore();

	private:
		struct impl;
		std::unique_ptr<impl> pImpl;
	};
}

using namespace CinderVk;

// getVkPtr<VkSurfaceKHR>();
// getVkPtr<vk::PhysicalDevice>();