#pragma once

#include <vector>
#include <span>
#include <vulkan/vulkan.h>

namespace baldwin {
namespace vk {

struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void addBinding(uint32_t binding, VkDescriptorType type,
		    uint32_t descriptorCount = 1);
    void clear();
    VkDescriptorSetLayout build(VkDevice device,
				VkShaderStageFlags shaderStages,
				void* pNext = nullptr,
				VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
	VkDescriptorType type;
	float ratio;
    };

    VkDescriptorPool pool;

    void initPool(VkDevice device, uint32_t maxSets,
		  std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout,
			     void* pNext = nullptr);
};

} // namespace vk
} // namespace baldwin
