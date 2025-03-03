#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/defines.h"

#include <vulkan/vulkan.h>

struct instance {
    VkInstance handle;
	VkDebugUtilsMessengerEXT debug_messenger;
};

struct render_system_state {
    struct instance instance;
};

#endif // RENDER_TYPES_H
