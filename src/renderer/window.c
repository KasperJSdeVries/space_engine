#include "window.h"

#include "core/result.h"
#include "types.h"
#include "vulkan/vulkan_xlib.h"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <string.h>

se_result window_create(Display *display, struct window *window) {
    memset(window, 0, sizeof(struct window));

    window->display = display;

    window->handle =
        XCreateSimpleWindow(display,
                            DefaultRootWindow(display),
                            0,
                            0,
                            800,
                            600,
                            0,
                            BlackPixel(display, DefaultScreen(display)),
                            BlackPixel(display, DefaultScreen(display)));

    u64 event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask |
                     KeyReleaseMask | ExposureMask | PointerMotionMask |
                     StructureNotifyMask;
    XSelectInput(display, window->handle, event_mask);

    window->wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window->handle, &window->wm_delete_window, 1);

    XMapWindow(display, window->handle);
    XFlush(display);

    return SE_RESULT_OK;
}

void window_destroy(struct window *window) {
    XUnmapWindow(window->display, window->handle);
    XDestroyWindow(window->display, window->handle);
}

se_result window_update(struct window *window) {
    i32 events_queued = XPending(window->display);
    if (events_queued == 0) {
        return SE_RESULT_OK;
    }

    XEvent event;
    if (XCheckTypedWindowEvent(window->display,
                               window->handle,
                               ConfigureNotify,
                               &event) == True) {
        return SE_RESULT_WINDOW_RESIZED;
    }

    if (XCheckTypedWindowEvent(window->display,
                               window->handle,
                               ClientMessage,
                               &event) == True) {
        if (memcmp(event.xclient.data.b,
                   &window->wm_delete_window,
                   sizeof(window->wm_delete_window)) == 0) {
            return SE_RESULT_WINDOW_CLOSED;
        }
    }

    return SE_RESULT_OK;
}

se_result window_surface_create(const struct window *window,
                                const struct instance *instance,
                                struct surface *surface) {
    VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = window->display,
        .window = window->handle,
    };

    if (vkCreateXlibSurfaceKHR(instance->handle,
                               &create_info,
                               NULL,
                               &surface->handle) != VK_SUCCESS) {
        return SE_RESULT_VULKAN_ERROR;
    }

    return SE_RESULT_OK;
}

void window_get_framebuffer_size(const struct window *window,
                                 VkExtent2D *extent) {
    XGetGeometry(window->display,
                 window->handle,
                 NULL,
                 NULL,
                 NULL,
                 &extent->width,
                 &extent->height,
                 NULL,
                 NULL);
}
