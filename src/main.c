#include "core/result.h"
#include "game.h"

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

    struct world world;

    world_setup(&world, &renderer);

    for (b8 quit = false; quit == false;) {
        /*XEvent event;*/
        /**/
        /*i32 events_queued = XPending(display);*/
        /*for (i32 i = 0; i < events_queued; i++) {*/
        /*    XNextEvent(display, &event);*/
        /*    switch (event.type) {*/
        /*    case Expose:*/
        /*        LOG_INFO("window expose");*/
        /*        break;*/
        /**/
        /*    case ConfigureNotify:*/
        /*        LOG_INFO("Resizing in Xlib");*/
        /*        for (u64 j = 0; j < state_ptr->windows.count; j++) {*/
        /*            if (event.xconfigure.window ==*/
        /*                state_ptr->windows.items[j]->platform_state->window)
         * {*/
        /*                struct se_window *window =
         * state_ptr->windows.items[j];*/
        /*                for (u64 k = 0; k < window->resize_callbacks.count;*/
        /*                     k++) {*/
        /*                    // xdd syntax*/
        /*                    window->resize_callbacks.items[k]();*/
        /*                }*/
        /*            }*/
        /*        }*/
        /*        break;*/
        /**/
        /*    case ClientMessage:*/
        /*        if (memcmp(event.xclient.data.b,*/
        /*                   &state_ptr->wm_delete_window,*/
        /*                   sizeof(state_ptr->wm_delete_window)) == 0) {*/
        /*            quit = true;*/
        /*        }*/
        /*        break;*/
        /*    }*/
        /*}*/

        se_result result;
        if ((result = renderer_start_frame(&renderer)) ==
            SE_RESULT_WINDOW_CLOSED) {
            quit = true;
            continue;
        } else if (result != SE_RESULT_OK) {
            LOG_ERROR("failed to start frame");
            quit = true;
        }

        world_update(&world, &renderer, 0.0f);

        if (renderer_end_frame(&renderer) != SE_RESULT_OK) {
            LOG_ERROR("failed to end frame");
            quit = true;
        }
    }

    renderer_end_main_loop(&renderer);

    world_cleanup(&world, &renderer);

    renderer_shutdown(&renderer);

    // HACK: display needs to be closed before instance is destroyed to prevent
    // segfault
    // (https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1894#issuecomment-309832783)
    XCloseDisplay(display);

    instance_destroy(&renderer.instance);
}
