#include <vulkan/vulkan.h>

namespace baldwin {
namespace vk {

VkImageSubresourceRange getImageSubresourceRange(VkImageAspectFlags aspectMask);
void createImageBarrier(VkCommandBuffer cmd, VkImage image,
			VkImageLayout imageLayout);
void createImageBarrierWithTransition(VkCommandBuffer cmd, VkImage image,
				      VkImageLayout currentLayout,
				      VkImageLayout newLayout);
void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination,
		      VkExtent2D srcSize, VkExtent2D dstSize);

} // namespace vk
} // namespace baldwin
