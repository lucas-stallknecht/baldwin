#pragma once

#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

#define VK_CHECK(x, message)                                                   \
    do {                                                                       \
	VkResult err = x;                                                      \
	if (err != VK_SUCCESS) {                                               \
	    printf("Detected Vulkan error: %s", string_VkResult(err));         \
	    throw std::runtime_error(message);                                 \
	}                                                                      \
    } while (0)
