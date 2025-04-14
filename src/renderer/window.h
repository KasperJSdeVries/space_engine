#ifndef RENDERER_WINDOW_H
#define RENDERER_WINDOW_H

#include "core/result.h"
#include "types.h"

#include <X11/Xlib.h>


se_result window_create(Display *display, struct window *window);
void window_destroy(struct window *window);
se_result window_update(struct window *window);

se_result window_surface_create(const struct window *window,
                                const struct instance *instance,
                                struct surface *surface);
void window_get_framebuffer_size(const struct window *window,
                                 VkExtent2D *extent);

#endif // RENDERER_WINDOW_H
