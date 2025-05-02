#include "client/box.h"
#include "core/result.h"

#include "core/assert.h"
#include "core/defines.h"
#include "renderer/instance.h"
#include "renderer/renderer.h"
#include "renderer/types.h"

#include "X11/Xlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    Display *display = XOpenDisplay(NULL);

    struct renderer renderer = {0};
    ASSERT(renderer_startup(display, &renderer) == SE_RESULT_OK);

    // struct world world;

    // world_setup(&world, &renderer);

    struct box box = box_new(&renderer);

    f32 time = 0.0f;

    for (b8 quit = false; quit == false;) {
        se_result result;
        if ((result = renderer_start_frame(&renderer)) ==
            SE_RESULT_WINDOW_CLOSED) {
            quit = true;
            continue;
        } else if (result != SE_RESULT_OK) {
            LOG_ERROR("failed to start frame");
            quit = true;
        }

        box_set_position(
            &box,
            (vec3s){{sinf(time * 0.5) * 5, cosf(time * 0.8), box.position.z}});
        box_set_scale(&box, vec3_scale(vec3_one(), sinf(time)));
        box_set_rotation(&box, (vec3s){{time * 0.2, time, time * 1.4}});

        box_render(&renderer, &box);
        // world_update(&world, &renderer, 0.0f);

        if (renderer_end_frame(&renderer) != SE_RESULT_OK) {
            LOG_ERROR("failed to end frame");
            quit = true;
        }

        time += 0.0001;
    }

    renderer_end_main_loop(&renderer);

    box_destroy(&renderer, &box);
    // world_cleanup(&world, &renderer);

    renderer_shutdown(&renderer);

    // HACK: display needs to be closed before instance is destroyed to prevent
    // segfault
    // (https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1894#issuecomment-309832783)
    XCloseDisplay(display);

    instance_destroy(&renderer.instance);
}
