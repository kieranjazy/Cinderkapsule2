#pragma once
#include <vector>
#include <optional>
#include <fstream>
#include "vulkan/vulkan.hpp"

namespace CinderVk {
	namespace Helper {
		struct SwapchainSupportDetails {
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR> presentModes;
		};

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		static SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice& device, vk::SurfaceKHR &surfaceKHR) {
			SwapchainSupportDetails details;
			device.getFeatures();
			details.capabilities = device.getSurfaceCapabilitiesKHR(surfaceKHR);

			uint32_t formatCount;
			device.getSurfaceFormatsKHR(surfaceKHR, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				device.getSurfaceFormatsKHR(surfaceKHR, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			device.getSurfacePresentModesKHR(surfaceKHR, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				device.getSurfacePresentModesKHR(surfaceKHR, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice &device, vk::SurfaceKHR& surfaceKHR) {
			QueueFamilyIndices indices;

			auto queueFamilies = device.getQueueFamilyProperties();

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (vk::QueueFlagBits::eGraphics & queueFamily.queueFlags)
					indices.graphicsFamily = i;

				VkBool32 presentSupport = false;
				device.getSurfaceSupportKHR(i, surfaceKHR, &presentSupport);

				if (presentSupport)
					indices.presentFamily = i;

				if (indices.isComplete())
					break;

				i++;
			}

			return indices;
		}

		static vk::ImageView createImageView(vk::Image& image, vk::Format& format, vk::ImageAspectFlags aspectFlags, vk::Device& device) {
			vk::ImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.image = image;
			viewCreateInfo.viewType = vk::ImageViewType::e2D;
			viewCreateInfo.format = format;
			viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			vk::ImageView imageView;

			if (device.createImageView(&viewCreateInfo, nullptr, &imageView) != vk::Result::eSuccess)
				throw std::runtime_error("Failed to create image view.");

			return imageView;
		}

		static vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features, vk::PhysicalDevice& physicalDevice) {
			for (vk::Format format : candidates) {
				vk::FormatProperties props;
				physicalDevice.getFormatProperties(format, &props);

				if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
					return format;
				}
				else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
					return format;
				}
			}

			throw std::runtime_error("Failed to find a supported format for the physical device.");
		}

		static vk::Format findDepthFormat(vk::PhysicalDevice& physicalDevice) {
			return findSupportedFormat(
				{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment,
				physicalDevice
			);
		}

		static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}
			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}
	}
}