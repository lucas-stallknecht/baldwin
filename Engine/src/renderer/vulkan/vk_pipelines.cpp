#include "vk_pipelines.hpp"

#include <cstdint>
#include <stdexcept>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#include "vk_infos.hpp"

namespace baldwin {
namespace vk {

void GraphicsPipelineBuilder::clear() {

    _inputAssembly = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };
    _rasterizer = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };
    _colorBlendAttachment = {};
    _multisampling = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
    };
    _pipelineLayout = {};
    _depthStencil = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    _renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

    _shaderStages.clear();
}

void GraphicsPipelineBuilder::setShaders(VkShaderModule vertexShader,
					 VkShaderModule fragmentShader) {
    _shaderStages.clear();
    _shaderStages.push_back(getPipelineShaderStageCreateInfo(
      VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
    _shaderStages.push_back(getPipelineShaderStageCreateInfo(
      VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void GraphicsPipelineBuilder::setInputTopology(VkPrimitiveTopology topology) {
    _inputAssembly.topology = topology;
    _inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    _rasterizer.polygonMode = mode;
    _rasterizer.lineWidth = 1.f;
}

void GraphicsPipelineBuilder::setCullMode(VkCullModeFlags cullMode,
					  VkFrontFace frontFace) {
    _rasterizer.cullMode = cullMode;
    _rasterizer.frontFace = frontFace;
}

void GraphicsPipelineBuilder::disableMultiSampling() {
    // 1 sample per pixel
    _multisampling.sampleShadingEnable = VK_FALSE;
    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f;
    _multisampling.pSampleMask = nullptr;
    _multisampling.alphaToCoverageEnable = VK_FALSE;
    _multisampling.alphaToOneEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::disableBlending() {
    _colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
					   VK_COLOR_COMPONENT_G_BIT |
					   VK_COLOR_COMPONENT_B_BIT |
					   VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::setColorAttachment(VkFormat format) {
    _colorAttachmentformat = format;
    _renderInfo.colorAttachmentCount = 1;
    _renderInfo.pColorAttachmentFormats = &_colorAttachmentformat;
}

void GraphicsPipelineBuilder::setDepthFormat(VkFormat format) {
    _renderInfo.depthAttachmentFormat = format;
}

void GraphicsPipelineBuilder::disableDepthTest() {
    _depthStencil.depthTestEnable = VK_FALSE;
    _depthStencil.depthWriteEnable = VK_FALSE;
    _depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    _depthStencil.depthBoundsTestEnable = VK_FALSE;
    _depthStencil.stencilTestEnable = VK_FALSE;
    _depthStencil.front = {};
    _depthStencil.back = {};
    _depthStencil.minDepthBounds = 0.0f;
    _depthStencil.maxDepthBounds = 1.0f;
}

VkPipeline GraphicsPipelineBuilder::build(const VkDevice& device) {
    // We use dynamic viewprt state so only counts are required
    VkPipelineViewportStateCreateInfo viewportInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	.viewportCount = 1,
	.scissorCount = 1
    };

    // We only render opaque objects for now
    // And to one attachment
    VkPipelineColorBlendStateCreateInfo blendInfo = {
	.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT,
	.logicOpEnable = VK_FALSE,
	.logicOp = VK_LOGIC_OP_COPY,
	.attachmentCount = 1,
	.pAttachments = &_colorBlendAttachment
    };

    VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
	.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	.pNext = &_renderInfo,
	.stageCount = static_cast<uint32_t>(_shaderStages.size()),
	.pStages = _shaderStages.data(),
	.pVertexInputState = &_vertexInputInfo,
	.pInputAssemblyState = &_inputAssembly,
	.pViewportState = &viewportInfo,
	.pRasterizationState = &_rasterizer,
	.pMultisampleState = &_multisampling,
	.pDepthStencilState = &_depthStencil,
	.pColorBlendState = &blendInfo,
	.layout = _pipelineLayout
    };

    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT,
			       VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	.dynamicStateCount = 2,
	.pDynamicStates = state
    };
    pipelineInfo.pDynamicState = &dynamicInfo;

    VkPipeline newPipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(
	  device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) !=
	VK_SUCCESS) {
	throw(std::runtime_error("Failed to create graphics pipeline"));
    }
    return newPipeline;
}

} // namespace vk
} // namespace baldwin
