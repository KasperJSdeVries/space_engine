#ifndef SE_PLATFORM_H
#define SE_PLATFORM_H

#include "core/defines.h"

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

typedef struct se_window {
    const char *title;

    struct se_window_platform_state *platform_state;
} se_window;

b8 platform_system_startup(u64 *memory_requirement, struct se_platform_state *state);

b8 platform_window_create(struct se_window_config *config, struct se_window *window);

#endif // SE_PLATFORM_H
