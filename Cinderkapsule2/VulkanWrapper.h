#pragma once
#include "VulkanCore.h"
#include "vulkan/vulkan.hpp"

namespace CinderVk {
	class VulkanWrapper { //Base class for Vulkan Core functionality classes
	public:
		VulkanWrapper(VulkanCore* coreRef) : corePtr(coreRef) {
			//init();
		}

		~VulkanWrapper() {
			//cleanup();
		}

		VulkanCore*& getCorePtr() {
			return corePtr;
		}

	private:
		VulkanCore* corePtr;

		virtual void init() = 0;
		virtual void cleanup() = 0;
	};
}
