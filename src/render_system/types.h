#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/defines.h"

#include <vulkan/vulkan.h>

#ifdef SE_DEBUG
#define ENABLE_VALIDATION_LAYERS true
#else
#define ENABLE_VALIDATION_LAYERS false
#endif

struct instance {
    VkInstance handle;
#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct device {
	VkPhysicalDevice physical_device;
	VkDevice handle;
};

struct render_system_state {
    struct instance instance;
	struct device device;
};

#endif // RENDER_TYPES_H
