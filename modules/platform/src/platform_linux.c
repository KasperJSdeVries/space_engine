#include "core/darray.h"
#include "core/defines.h"
#include "platform.h"
#include "vulkan/vulkan_core.h"

#ifdef __linux__
#include "core/assert.h"
#include "core/logging.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct window_array {
    struct se_window **items;
    u64 count;
    u64 capacity;
};

typedef struct se_platform_state {
    Display *display;
    struct window_array windows;
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
    memset(state_ptr, 0, sizeof(se_platform_state));
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

        case ConfigureNotify:
            LOG_INFO("Resizing in Xlib");
            for (u64 j = 0; j < state_ptr->windows.count; j++) {
                if (event.xconfigure.window ==
                    state_ptr->windows.items[j]->platform_state->window) {
                    struct se_window *window = state_ptr->windows.items[j];
                    for (u64 k = 0; k < window->resize_callbacks.count; k++) {
                        // xdd syntax
                        window->resize_callbacks.items[k]();
                    }
                }
            }
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

    memset(window, 0, sizeof(struct se_window));
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

    darray_append(&state_ptr->windows, window);

    return true;
}

void platform_window_destroy(struct se_window *window) {
    LOG_INFO("Closing Window");
    XUnmapWindow(state_ptr->display, window->platform_state->window);
    XDestroyWindow(state_ptr->display, window->platform_state->window);
    free(window->platform_state);
}

void platform_window_register_resize_callback(struct se_window *window,
                                              window_resized_callback_t callback) {
    darray_append(&window->resize_callbacks, callback);
}

static const char *linux_required_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME,
                                                  VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
static const u32 linux_required_extension_count =
    sizeof(linux_required_extensions) / sizeof(*linux_required_extensions);

void platform_get_required_extensions(u32 *extension_count, const char **required_extensions) {
    if (extension_count != NULL) {
        *extension_count = linux_required_extension_count;
    }

    if (required_extensions != NULL) {
        for (u32 i = 0; i < linux_required_extension_count; i++) {
            required_extensions[i] = linux_required_extensions[i];
        }
    }
}

b8 platform_surface_create(const struct se_window *window,
                           VkInstance instance,
                           VkSurfaceKHR *surface) {
    VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = state_ptr->display,
        .window = window->platform_state->window,
    };

    if (!ASSERT(vkCreateXlibSurfaceKHR(instance, &create_info, NULL, surface) == VK_SUCCESS)) {
        return false;
    }

    return true;
}

void platform_get_framebuffer_size(const struct se_window *window, VkExtent2D *extent) {
    XGetGeometry(state_ptr->display,
                 window->platform_state->window,
                 NULL,
                 NULL,
                 NULL,
                 &extent->width,
                 &extent->height,
                 NULL,
                 NULL);
}

#endif // __linux__
