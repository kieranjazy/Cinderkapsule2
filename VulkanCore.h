#pragma once
#include <memory>

namespace vk {
	class PhysicalDevice;
	class Device;
	class SurfaceKHR;
	enum class Format;
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