#include <vulkan/vulkan.h>

namespace baldwin {
namespace vk {

VkImageSubresourceRange getImageSubresourceRange(VkImageAspectFlags aspectMask);
void transitionImage(VkCommandBuffer cmd, VkImage image,
		     VkImageLayout currentLayout, VkImageLayout newLayout);
} // namespace vk
} // namespace baldwin
