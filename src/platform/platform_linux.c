#include "platform.h"

#ifdef __linux__
#include "core/assert.h"
#include "core/logging.h"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct se_platform_state {
    Display *display;
    Atom wm_delete_window;
} se_platform_state;

static se_platform_state *state_ptr;

struct se_window_platform_state {
    Window window;
};

b8 platform_system_startup(size_t *memory_requirement, se_platform_state *state) {
    if (!ASSERT(memory_requirement != NULL)) {
        return false;
    }

    *memory_requirement = sizeof(se_platform_state);
    if (state == NULL) {
        return true;
    }

    state_ptr = state;
    state_ptr->display = XOpenDisplay(NULL);

    if (!ASSERT(state_ptr->display)) {
        return false;
    }

    return true;
}

b8 platform_system_poll(struct se_platform_state *state) {
    XEvent event;

    i32 events_queued = XPending(state->display);
    for (i32 i = 0; i < events_queued; i++) {
        XNextEvent(state->display, &event);
        switch (event.type) {
        case Expose:
            LOG_INFO("window expose");
            break;

        case ClientMessage:
            if (memcmp(event.xclient.data.b,
                       &state_ptr->wm_delete_window,
                       sizeof(state_ptr->wm_delete_window)) == 0) {
                return false;
            }
            break;
        }
    }

    return true;
}

void platform_system_shutdown(struct se_platform_state *state) {
    if (state != NULL) {
        XCloseDisplay(state->display);
    }
    state_ptr = NULL;
}

b8 platform_window_create(struct se_window_config *config, struct se_window *window) {
    if (!ASSERT(window)) {
        return false;
    }

    window->platform_state = malloc(sizeof(struct se_window_platform_state));

    window->platform_state->window =
        XCreateSimpleWindow(state_ptr->display,
                            DefaultRootWindow(state_ptr->display),
                            config->position_x,
                            config->position_y,
                            config->width,
                            config->height,
                            0,
                            BlackPixel(state_ptr->display, DefaultScreen(state_ptr->display)),
                            BlackPixel(state_ptr->display, DefaultScreen(state_ptr->display)));

    u64 event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                     ExposureMask | PointerMotionMask | StructureNotifyMask;
    XSelectInput(state_ptr->display, window->platform_state->window, event_mask);

    state_ptr->wm_delete_window = XInternAtom(state_ptr->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state_ptr->display,
                    window->platform_state->window,
                    &state_ptr->wm_delete_window,
                    1);

    XMapWindow(state_ptr->display, window->platform_state->window);
    XFlush(state_ptr->display);

    return true;
}

void platform_window_destroy(struct se_window *window) {
    LOG_INFO("Closing Window");
    XUnmapWindow(state_ptr->display, window->platform_state->window);
    XDestroyWindow(state_ptr->display, window->platform_state->window);
    free(window->platform_state);
}

#endif // __linux__
