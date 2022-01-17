#pragma once
#include "VulkanWrapper.h"
#include "VulkanHelper.h"
#include "Vertex.h"
#include <vector>

namespace CinderVk {
	class VulkanGraphicsPipeline : VulkanWrapper {
	public:
		VulkanGraphicsPipeline(VulkanCore* corePtr) : VulkanWrapper(corePtr) {
			init();
		}

		~VulkanGraphicsPipeline() {
			cleanup();
		}

	private:
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline graphicsPipeline;

		void init() {
			std::vector<char> vertexShaderCode = Helper::readFile("vertexShader.spv"); //TODO make these files again
			std::vector<char> fragmentShaderCode = Helper::readFile("fragmentShader.spv");

			vk::ShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
			vk::ShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

			addShaderStageInfo(vk::ShaderStageFlagBits::eVertex, vertexShaderModule);
			addShaderStageInfo(vk::ShaderStageFlagBits::eFragment, fragmentShaderModule);

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
			
			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();

			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.topology = vk::PrimitiveTopology::eTriangleList; //could be fun to mess with
			inputAssembly.primitiveRestartEnable = 0;

			vk::Viewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float) getCorePtr()->getSwapchainExtentWidth();
			viewport.height = (float) getCorePtr()->getSwapchainExtentHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			vk::Rect2D scissor{};
			scissor.offset = vk::Offset2D(0, 0);
			scissor.extent = getCorePtr()->getSwapchainExtent();

			vk::PipelineViewportStateCreateInfo viewportState{};
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			vk::PipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.depthClampEnable = 0;
			rasterizer.rasterizerDiscardEnable = 0;
			rasterizer.polygonMode = vk::PolygonMode::eFill;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = vk::CullModeFlagBits::eNone;
			rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
			rasterizer.depthBiasEnable = 0;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasSlopeFactor = 0.0f;

			vk::PipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sampleShadingEnable = 0;
			multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = 0;
			multisampling.alphaToOneEnable = 0;

			vk::PipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.depthTestEnable = 1;
			depthStencil.depthWriteEnable = 1;
			depthStencil.depthCompareOp = vk::CompareOp::eLess;
			depthStencil.depthBoundsTestEnable = 0;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;
			depthStencil.stencilTestEnable = 0;

			vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA;
			colorBlendAttachment.blendEnable = 1;
			colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
			colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
			colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
			colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
			colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

			vk::PipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.logicOpEnable = 0;
			colorBlending.logicOp = vk::LogicOp::eCopy;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.setLayoutCount = 1;

			vk::DescriptorSetLayout&& layouts = getCorePtr()->getDescriptorSetLayout();

			pipelineLayoutInfo.pSetLayouts = &layouts;
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			if (getCorePtr()->getLogicalDevicePtr()->createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to create the pipeline layout.");
			}

			vk::GraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = nullptr;

			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = getCorePtr()->getRenderPass();
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = nullptr;
			pipelineInfo.basePipelineIndex = -1;

			if (getCorePtr()->getLogicalDevicePtr()->createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to create graphics pipeline.");
			}

			getCorePtr()->getLogicalDevicePtr()->destroyShaderModule(fragmentShaderModule, nullptr);
			getCorePtr()->getLogicalDevicePtr()->destroyShaderModule(vertexShaderModule, nullptr);
		}

		void cleanup() {
			getCorePtr()->getLogicalDevicePtr()->destroyPipeline(graphicsPipeline, nullptr);
			getCorePtr()->getLogicalDevicePtr()->destroyPipelineLayout(pipelineLayout, nullptr);
		}

		void addShaderStageInfo(vk::ShaderStageFlagBits flagBits, vk::ShaderModule shaderModule, const char* pName = "main") {
			vk::PipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.stage = flagBits;
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = pName;

			shaderStages.push_back(shaderStageInfo);
		}

		vk::ShaderModule createShaderModule(const std::vector<char>& shaderCode) {
			vk::ShaderModuleCreateInfo smCreateInfo{};

			smCreateInfo.codeSize = shaderCode.size();
			smCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

			vk::ShaderModule shaderModule;
			if (getCorePtr()->getLogicalDevicePtr()->createShaderModule(&smCreateInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to create a shader module.");
			}

			return shaderModule;
		}

	};



}