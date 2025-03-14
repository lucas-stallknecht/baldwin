#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace baldwin {
namespace vk {

VkSemaphoreSubmitInfo getSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask,
					     VkSemaphore semaphore);
VkCommandBufferBeginInfo getCommandBufferBeginInfo(
  VkCommandBufferUsageFlags flags);
VkCommandBufferSubmitInfo getCommandBufferSubmitInfo(VkCommandBuffer cmd);
VkSubmitInfo2 getSubmitInfo(VkCommandBufferSubmitInfo* cmd,
			    VkSemaphoreSubmitInfo* signalSemaphoreInfo,
			    VkSemaphoreSubmitInfo* waitSemaphoreInfo);
VkRenderingAttachmentInfo getAttachmentInfo(VkImageView view,
					    VkClearValue* clear,
					    VkImageLayout layout);
VkRenderingInfo getRenderingInfo(VkExtent2D renderExtent,
				 VkRenderingAttachmentInfo* colorAttachment,
				 VkRenderingAttachmentInfo* depthAttachment);
VkImageCreateInfo getImageCreateInfo(VkFormat format,
				     VkImageUsageFlags usageFlags,
				     VkExtent3D extent, bool cubeMap = false);
VkImageViewCreateInfo getImageViewCreateInfo(VkFormat format, VkImage iamge,
					     VkImageAspectFlags aspectFlags,
					     bool cubeMap = false);
VkPipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo(
  VkShaderStageFlagBits stage, VkShaderModule module,
  const char* name = "main");
} // namespace vk
} // namespace baldwin
