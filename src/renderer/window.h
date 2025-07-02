#ifndef RENDERER_WINDOW_H
#define RENDERER_WINDOW_H

#include "containers/darray.h"
#include "vulkan.h"

typedef struct Window Window;

typedef struct {
    const char *title;
    u32 width;
    u32 height;
    b8 cursor_disabled;
    b8 fullscreen;
    b8 resizable;
} WindowConfig;

Window *window_new(WindowConfig config);
void window_destroy(Window *window);

GLFWwindow *window_handle(const Window *window);
WindowConfig window_config(const Window *window);
f32 window_content_scale(const Window *window);
VkExtent2D window_framebuffer_size(const Window *window);
VkExtent2D window_size(const Window *window);

const char *window_get_key_name(i32 key, i32 scancode);
darray(const char *) window_get_required_instance_extensions(void);
f64 window_get_time(void);

void window_close(Window *window);
b8 window_is_minimized(const Window *window);
void window_run(Window *window);
void window_wait_for_events(void);

void window_set_context(Window *window, void *ctx);
void window_set_draw_frame(Window *window, void (*draw_frame)(void *ctx));
void window_set_on_key(
    Window *window,
    void (*on_key)(void *context, i32 key, i32 scancode, i32 action, i32 mods));
void window_set_on_cursor_position(Window *window,
                                   void (*on_cursor_position)(void *ctx,
                                                              f64 x_pos,
                                                              f64 y_pos));
void window_set_on_mouse_button(
    Window *window,
    void (*on_mouse_button)(void *ctx, i32 button, i32 action, i32 mods));
void window_set_on_scroll(Window *window,
                          void (*on_scroll)(void *ctx,
                                            f64 x_offset,
                                            f64 y_offset));

// se_result window_surface_create(const struct window *window,
//                                 const struct instance *instance,
//                                 struct surface *surface);

#endif // RENDERER_WINDOW_H
