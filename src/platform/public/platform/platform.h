#ifndef SE_PLATFORM_H
#define SE_PLATFORM_H

#include "core/defines.h"
#include <vulkan/vulkan.h>

struct se_platform_state;

struct se_window_config {
    i32 position_x;
    i32 position_y;
    u32 width;
    u32 height;
    const char *title;
    const char *name;
    b8 full_screen;
};

struct se_window_platform_state;

typedef void (*window_resized_callback_t)(void);
struct resize_callbacks_array {
    window_resized_callback_t *items;
    u64 count;
    u64 capacity;
};

typedef struct se_window {
    const char *title;
    struct resize_callbacks_array resize_callbacks;
    struct se_window_platform_state *platform_state;
} se_window;

b8 platform_system_startup(u64 *memory_requirement, struct se_platform_state *state);
b8 platform_system_poll(struct se_platform_state *state);
void platform_system_shutdown(struct se_platform_state *state);

b8 platform_window_create(struct se_window_config *config, struct se_window *window);
void platform_window_destroy(struct se_window *window);
void platform_window_register_resize_callback(struct se_window *window,
                                              window_resized_callback_t callback);

void platform_get_required_extensions(u32 *extension_count, const char **required_extensions);
b8 platform_surface_create(const struct se_window *window,
                           VkInstance instance,
                           VkSurfaceKHR *surface);
void platform_get_framebuffer_size(const struct se_window *window, VkExtent2D *extent);

#endif // SE_PLATFORM_H
