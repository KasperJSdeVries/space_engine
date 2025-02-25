#ifndef SE_PLATFORM_H
#define SE_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

struct se_platform_state;

struct se_window_config {
    int32_t position_x;
    int32_t position_y;
    uint32_t width;
    uint32_t height;
    const char *title;
    const char *name;
    bool full_screen;
};

struct se_window_platform_state;

typedef struct se_window {
    const char *title;

    struct se_window_platform_state *platform_state;
} se_window;

bool platform_system_startup(size_t *memory_requirement, struct se_platform_state *state);

bool platform_window_create(struct se_window_config *config, struct se_window *window);

#endif // SE_PLATFORM_H
