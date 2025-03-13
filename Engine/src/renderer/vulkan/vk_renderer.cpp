#include <iostream>
#define VMA_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include "vk_renderer.hpp"

#include <cmath>
#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>
#include "graphics_macros.hpp"
#include "vk_images.hpp"
#include "vk_infos.hpp"
#include "renderer/vulkan/vk_shaders.hpp"

namespace baldwin {
namespace vk {

bool VulkanRenderer::init(GLFWwindow* window, int width, int height,
			  bool tripleBuffering) {
    if (tripleBuffering)
	_frameOverlap = 3;

    initVulkan(window);
    createSwapchain(width, height);
    createCommands();
    createSync();
    initDescriptors();
    initBackgroundPipeline();

    return true;
}

void VulkanRenderer::initVulkan(GLFWwindow* window) {
    /*  For an example of initialization without using VKB, see the Vulkan
     path tracer source code */

    // Instance
    vkb::InstanceBuilder builder;

#ifdef ENABLE_VALIDATION_LAYERS
    auto instRes = builder.set_app_name("Baldwin Engine Application")
		     .request_validation_layers(VALIDATIONS_LAYERS.data())
		     .use_default_debug_messenger()
		     .require_api_version(1, 3, 0)
		     .build();
#else
    auto instRes = builder.set_app_name("Baldwin Engine Application")
		     .require_api_version(1, 3, 0)
		     .build();
#endif
    vkb::Instance vkbInst = instRes.value();
    _instance = vkbInst.instance;

    // Surface
    glfwCreateWindowSurface(_instance, window, nullptr, &_surface);

    // Physical and logical devices
    VkPhysicalDeviceVulkan13Features features13 = {
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
    };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12 = {
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
    };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{ vkbInst };
    vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
					   .set_required_features_13(features13)
					   .set_required_features_12(features12)
					   .set_surface(_surface)
					   .select()
					   .value();
    std::cout << "Selected GPU :" << physicalDevice.name << std::endl;
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _gpu = vkbDevice.physical_device;

    // Queue and queue family
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics)
			     .value();
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

    // VMA
    VmaAllocatorCreateInfo allocatorInfo = {
	.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
	.physicalDevice = _gpu,
	.device = _device,
	.instance = _instance,
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &_allocator),
	     "Could not create VMA Allocator");

    _deletionQueue.pushFunction([this, vkbInst]() {
	vmaDestroyAllocator(_allocator);
	vkb::destroy_debug_utils_messenger(_instance, vkbInst.debug_messenger);
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyDevice(_device, nullptr);
	vkDestroyInstance(_instance, nullptr);
    });
}

void VulkanRenderer::createSwapchain(int width, int height) {
    _swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::SwapchainBuilder swapchainBuilder{ _gpu, _device, _surface };
    vkb::Swapchain
      vkbSwapchain = swapchainBuilder
		       //.use_default_format_selection()
		       .set_desired_format(VkSurfaceFormatKHR{
			 .format = _swapchainFormat,
			 .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		       // use vsync present mode
		       .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		       .set_desired_extent(width, height)
		       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		       .build()
		       .value();

    _swapchainExtent = vkbSwapchain.extent;
    // store swapchain and its related imagessnip-next-choice
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();

    // Draw image
    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    _drawImage.imageExtent = { _swapchainExtent.width,
			       _swapchainExtent.height,
			       1 };

    VkImageUsageFlags drawImageUsages = {};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo imgInfo = getImageCreateInfo(
      _drawImage.imageFormat, drawImageUsages, _drawImage.imageExtent);

    // GPU memory only
    VmaAllocationCreateInfo imgAllocInfo = {
	.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	.requiredFlags = VkMemoryPropertyFlags(
	  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    vmaCreateImage(_allocator,
		   &imgInfo,
		   &imgAllocInfo,
		   &_drawImage.image,
		   &_drawImage.allocation,
		   nullptr);

    // Associated view
    VkImageViewCreateInfo imgViewInfo = getImageViewCreateInfo(
      _drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
    VK_CHECK(
      vkCreateImageView(_device, &imgViewInfo, nullptr, &_drawImage.imageView),
      "Could not create draw iamge view");

    _deletionQueue.pushFunction([this]() {
	vkDestroyImageView(_device, _drawImage.imageView, nullptr);
	vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
	for (auto& imgView : _swapchainImageViews) {
	    vkDestroyImageView(_device, imgView, nullptr);
	}
    });
}

void VulkanRenderer::createCommands() {
    // Frames
    VkCommandPoolCreateInfo poolInfo = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	.queueFamilyIndex = _graphicsQueueFamily
    };

    for (int i = 0; i < _frameOverlap; i++) {
	FrameData frame{};
	VK_CHECK(
	  vkCreateCommandPool(_device, &poolInfo, nullptr, &frame.commandPool),
	  "Could not create frame command pool");

	VkCommandBufferAllocateInfo bufferInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .commandPool = frame.commandPool,
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandBufferCount = 1,
	};
	VK_CHECK(vkAllocateCommandBuffers(
		   _device, &bufferInfo, &frame.mainCommandBuffer),
		 "Could not allocate frame main command buffer");

	_frames.push_back(frame);
	_frames[i].deletionQueue.pushFunction([this, i]() {
	    vkFreeCommandBuffers(_device,
				 _frames[i].commandPool,
				 1,
				 &_frames[i].mainCommandBuffer);
	    vkDestroyCommandPool(_device, _frames[i].commandPool, nullptr);
	});
    }

    // Immediate commands (soon)
}

void VulkanRenderer::createSync() {
    VkSemaphoreCreateInfo semaphoreInfo = {
	.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceInfo = {
	.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	.flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (int i = 0; i < _frameOverlap; i++) {
	VK_CHECK(vkCreateSemaphore(
		   _device, &semaphoreInfo, nullptr, &_frames[i].swapSemaphore),
		 "Could not create frame swapSemaphore");
	VK_CHECK(
	  vkCreateSemaphore(
	    _device, &semaphoreInfo, nullptr, &_frames[i].renderSemaphore),
	  "Could not create frame renderSemaphore");
	VK_CHECK(
	  vkCreateFence(_device, &fenceInfo, nullptr, &_frames[i].renderFence),
	  "Could not create frame renderFence");

	_frames[i].deletionQueue.pushFunction([this, i]() {
	    vkDestroyFence(_device, _frames[i].renderFence, nullptr);
	    vkDestroySemaphore(_device, _frames[i].renderSemaphore, nullptr);
	    vkDestroySemaphore(_device, _frames[i].swapSemaphore, nullptr);
	});
    }
}

void VulkanRenderer::initDescriptors() {
    std::vector<DescriptorAllocator::PoolSizeRatio> poolSizes = {
	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };
    _descriptorAllocator.initPool(_device, 10, poolSizes);

    DescriptorLayoutBuilder builder{};
    builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    _bgSetLayout = builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
    _bgDescriptors = _descriptorAllocator.allocate(
      _device, _bgSetLayout, nullptr);

    VkDescriptorImageInfo imgWriteInfo = {
	.imageView = _drawImage.imageView,
	.imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };
    VkWriteDescriptorSet writeInfo = {
	.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	.dstSet = _bgDescriptors,
	.dstBinding = 0,
	.descriptorCount = 1,
	.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	.pImageInfo = &imgWriteInfo
    };
    vkUpdateDescriptorSets(_device, 1, &writeInfo, 0, nullptr);

    _deletionQueue.pushFunction([this]() {
	vkDestroyDescriptorSetLayout(_device, _bgSetLayout, nullptr);
	_descriptorAllocator.destroyPool(_device);
    });
}

void VulkanRenderer::initBackgroundPipeline() {
    VkPipelineLayoutCreateInfo ppLayoutInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	.setLayoutCount = 1,
	.pSetLayouts = &_bgSetLayout
    };
    VK_CHECK(vkCreatePipelineLayout(
	       _device, &ppLayoutInfo, nullptr, &_bgPipelineLayout),
	     "Could not create background pipeline layout");

    auto bgCode = readFile("shaders/test.comp.spv");
    VkShaderModule module = createShaderModule(_device, bgCode);
    VkPipelineShaderStageCreateInfo stageInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	.stage = VK_SHADER_STAGE_COMPUTE_BIT,
	.module = module,
	.pName = "main",
    };
    VkComputePipelineCreateInfo ppInfo = {
	.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
	.stage = stageInfo,
	.layout = _bgPipelineLayout
    };
    VK_CHECK(vkCreateComputePipelines(
	       _device, nullptr, 1, &ppInfo, nullptr, &_bgPipeline),
	     "Could not create background pipeline");

    vkDestroyShaderModule(_device, module, nullptr);
    _deletionQueue.pushFunction([this]() {
	vkDestroyPipelineLayout(_device, _bgPipelineLayout, nullptr);
	vkDestroyPipeline(_device, _bgPipeline, nullptr);
    });
}

void VulkanRenderer::run(int frameNum) { draw(frameNum); }

void VulkanRenderer::draw(int frameNum) {
    // Wait for GPU to finish rendering
    vkWaitForFences(
      _device, 1, &getCurrentFrame(frameNum).renderFence, VK_TRUE, 1000000000);
    vkResetFences(_device, 1, &getCurrentFrame(frameNum).renderFence);

    // Request swapchain image index that we can blit on
    uint32_t swapchainImgIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device,
				   _swapchain,
				   1000000,
				   getCurrentFrame(frameNum).swapSemaphore,
				   nullptr,
				   &swapchainImgIndex),
	     "Could not get swap chain image index");

    // Usual command workflow is : 1. wait / 2. reset / 3. begin / 4. record
    // / 5. submit to queue
    VkCommandBuffer cmd = getCurrentFrame(frameNum).mainCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0), "Could not reset command buffer");
    VkCommandBufferBeginInfo cmdBeginInfo = getCommandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo),
	     "Could not begin command recording");

    createImageBarrierWithTransition(cmd,
				     _drawImage.image,
				     VK_IMAGE_LAYOUT_UNDEFINED,
				     VK_IMAGE_LAYOUT_GENERAL);

    float bValue = 0.5 + 0.5 * std::sin(static_cast<float>(frameNum) / 120.0);
    VkClearColorValue clearValue = { 0.0, 0.0, bValue, 1.0 };
    VkImageSubresourceRange srcRange = getImageSubresourceRange(
      VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd,
			 _drawImage.image,
			 VK_IMAGE_LAYOUT_GENERAL,
			 &clearValue,
			 1,
			 &srcRange);

    createImageBarrier(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _bgPipeline);
    vkCmdBindDescriptorSets(cmd,
			    VK_PIPELINE_BIND_POINT_COMPUTE,
			    _bgPipelineLayout,
			    0,
			    1,
			    &_bgDescriptors,
			    0,
			    nullptr);
    vkCmdDispatch(cmd,
		  std::ceil(_drawImage.imageExtent.width / 16.0),
		  std::ceil(_drawImage.imageExtent.height / 16.0),
		  1);

    // Copy draw image content onto the swap chain image
    createImageBarrierWithTransition(cmd,
				     _drawImage.image,
				     VK_IMAGE_LAYOUT_GENERAL,
				     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    createImageBarrierWithTransition(cmd,
				     _swapchainImages[swapchainImgIndex],
				     VK_IMAGE_LAYOUT_UNDEFINED,
				     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyImageToImage(cmd,
		     _drawImage.image,
		     _swapchainImages[swapchainImgIndex],
		     _swapchainExtent,
		     _swapchainExtent);
    createImageBarrierWithTransition(cmd,
				     _swapchainImages[swapchainImgIndex],
				     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd), "Could not end command recording");

    // We finished drawing, time to submit
    VkCommandBufferSubmitInfo cmdSubmitInfo = getCommandBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = getSemaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      getCurrentFrame(frameNum).swapSemaphore);
    VkSemaphoreSubmitInfo signalInfo = getSemaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
      getCurrentFrame(frameNum).renderSemaphore);
    VkSubmitInfo2 submitInfo = getSubmitInfo(
      &cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(
      vkQueueSubmit2(
	_graphicsQueue, 1, &submitInfo, getCurrentFrame(frameNum).renderFence),
      "Could not submit graphics commands to queue");

    // We wait for rendering operations to finish and we
    // present
    VkPresentInfoKHR presentInfo = {
	.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	.pNext = nullptr,
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = &getCurrentFrame(frameNum).renderSemaphore,
	.swapchainCount = 1,
	.pSwapchains = &_swapchain,
	.pImageIndices = &swapchainImgIndex
    };
    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo),
	     "Could not present");
}

void VulkanRenderer::cleanup() {
    vkDeviceWaitIdle(_device);
    for (auto& frame : _frames) {
	frame.deletionQueue.flush();
    }
    _deletionQueue.flush();
}

} // namespace vk
} // namespace baldwin
