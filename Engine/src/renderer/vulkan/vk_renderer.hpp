#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <deque>
#include <functional>
#include <vulkan/vulkan.h>

#include "../renderer.hpp"

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

class VulkanRenderer : public Renderer {
  public:
    bool init(GLFWwindow* window, int width, int height,
	      bool tripleBuffering) override;
    void run(int frame) override;
    void cleanup() override;

  private:
    void initVulkan(GLFWwindow* window);
    void createSwapchain(int width, int height);
    void createCommands();
    void createSync();
    void draw(int frame);

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

    DeletionQueue _deletionQueue;
    std::vector<FrameData> _frames;
    int _frameOverlap = 2;
    FrameData& getCurrentFrame(int frameNum) {
	return _frames[frameNum % _frameOverlap];
    }
};

} // namespace vk
} // namespace baldwin
