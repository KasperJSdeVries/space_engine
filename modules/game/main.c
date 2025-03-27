#include "game.h"

#include "core/assert.h"
#include "core/defines.h"
#include "platform/platform.h"
#include "render_system/render.h"
#include "render_system/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    u64 platform_state_size = 0;
    platform_system_startup(&platform_state_size, NULL);
    struct se_platform_state *platform_state = malloc(platform_state_size);
    platform_system_startup(&platform_state_size, platform_state);

    struct se_window_config window_config = {
        .width = 720,
        .height = 480,
    };
    struct se_window window;
    platform_window_create(&window_config, &window);

    struct renderer renderer = {0};
    ASSERT(render_system_startup(&window, &renderer));

    struct world world;

    world_setup(&world, &renderer);

    for (b8 quit = false; quit == false;) {
        if (!platform_system_poll(platform_state)) {
            quit = true;
        }

        if (!render_system_start_frame(&window, &renderer)) {
            quit = true;
        }

        world_update(&world, &renderer, 0.0f);

        if (!render_system_end_frame(&window, &renderer)) {
            quit = true;
        }
    }

    render_system_end_main_loop(&renderer);

    world_cleanup(&world, &renderer);

    render_system_shutdown(&renderer);

    platform_window_destroy(&window);

    platform_system_shutdown(platform_state);
    free(platform_state);
}
