#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "../renderer.hpp"
#include "renderer/vulkan/vk_descriptors.hpp"

namespace baldwin {
namespace vk {

const std::vector<const char*> VALIDATIONS_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void pushFunction(std::function<void()>&& function) {
	deletors.push_back(function);
    }

    void flush() {
	// reverse iterate the deletion queue to execute all the functions
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
	    (*it)(); // call the function
	}
	deletors.clear();
    }
};

struct FrameData {
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer mainCommandBuffer = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;
    VkSemaphore swapSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    DeletionQueue deletionQueue;
};

struct AllocatedImage {
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkExtent3D imageExtent = {};
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
};

class VulkanRenderer : public Renderer {
  public:
    bool init(GLFWwindow* window, int width, int height,
	      bool tripleBuffering) override;
    void run(int frameNum) override;
    void cleanup() override;

  private:
    void initVulkan(GLFWwindow* window);
    void createSwapchain(int width, int height);
    void createCommands();
    void createSync();
    void initDescriptors();
    void initBackgroundPipeline();
    void clear(const VkCommandBuffer& cmd, int frameNum);
    void draw(int frameNum);

    // General
    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _gpu = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkFormat _swapchainFormat = VK_FORMAT_UNDEFINED;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkExtent2D _swapchainExtent = { 0, 0 };
    VkQueue _graphicsQueue = VK_NULL_HANDLE;
    uint32_t _graphicsQueueFamily;
    VmaAllocator _allocator{};

    // Resources
    AllocatedImage _drawImage{};
    VkDescriptorSetLayout _bgSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet _bgDescriptors = VK_NULL_HANDLE;
    VkPipelineLayout _bgPipelineLayout = VK_NULL_HANDLE;
    VkPipeline _bgPipeline = VK_NULL_HANDLE;

    // Helpers
    DeletionQueue _deletionQueue;
    DescriptorAllocator _descriptorAllocator{};
    std::vector<FrameData> _frames;
    int _frameOverlap = 2;
    FrameData& getCurrentFrame(int frameNum) {
	return _frames[frameNum % _frameOverlap];
    }
};

} // namespace vk
} // namespace baldwin
