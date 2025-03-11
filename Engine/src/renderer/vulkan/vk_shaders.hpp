#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

namespace baldwin {
namespace vk {

std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(VkDevice device,
				  const std::vector<char>& code);

} // namespace vk
} // namespace baldwin
