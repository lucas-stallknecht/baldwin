#include "vk_shaders.hpp"

#include <fstream>
#include <format>
#include "graphics_macros.hpp"

namespace baldwin {
namespace vk {

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
	throw std::runtime_error(
	  std::format("Failed to open file {}", filename));
    }

    std::streamsize fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device,
				  const std::vector<char>& code) {
    VkShaderModule module;
    VkShaderModuleCreateInfo createInfo = {
	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	.codeSize = code.size(),
	.pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &module),
	     "Could not create shader module!");

    return module;
}
} // namespace vk
} // namespace baldwin
