#include "window.h"

#include "core/defines.h"
#include "core/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Window {
    WindowConfig config;
    GLFWwindow *window;
    void *context;
    void (*draw_frame)(void *);
    void (*on_key)(void *, i32, i32, i32, i32);
    void (*on_cursor_position)(void *, f64, f64);
    void (*on_mouse_button)(void *, i32, i32, i32);
    void (*on_scroll)(void *, f64, f64);
};

static void glfw_error_callback(i32 error, const char *description) {
    LOG_ERROR("GLFW: %s (code: %d)", description, error);
}

static void glfw_key_callback(GLFWwindow *window,
                              i32 key,
                              i32 scancode,
                              i32 action,
                              i32 mods) {
    Window *this = (Window *)glfwGetWindowUserPointer(window);
    if (this->on_key != NULL) {
        this->on_key(this->context, key, scancode, action, mods);
    }
}

static void glfw_cursor_position_callback(GLFWwindow *window,
                                          f64 x_pos,
                                          f64 y_pos) {
    Window *this = (Window *)glfwGetWindowUserPointer(window);
    if (this->on_cursor_position != NULL) {
        this->on_cursor_position(this->context, x_pos, y_pos);
    }
}

static void glfw_mouse_button_callback(GLFWwindow *window,
                                       i32 button,
                                       i32 action,
                                       i32 mods) {
    Window *this = (Window *)glfwGetWindowUserPointer(window);
    if (this->on_mouse_button != NULL) {
        this->on_mouse_button(this->context, button, action, mods);
    }
}

static void glfw_scroll_callback(GLFWwindow *window,
                                 f64 x_offset,
                                 f64 y_offset) {
    Window *this = (Window *)glfwGetWindowUserPointer(window);
    if (this->on_scroll != NULL) {
        this->on_scroll(this->context, x_offset, y_offset);
    }
}

Window *window_new(WindowConfig config) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        LOG_FATAL("glfwInit() failed");
        exit(EXIT_FAILURE);
    }

    if (!glfwVulkanSupported()) {
        LOG_FATAL("glfwVulkanSupported() failed");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor *monitor = config.fullscreen ? glfwGetPrimaryMonitor() : NULL;

    GLFWwindow *window = glfwCreateWindow(config.width,
                                          config.height,
                                          config.title,
                                          monitor,
                                          NULL);
    if (window == NULL) {
        LOG_FATAL("failed to create window");
        exit(EXIT_FAILURE);
    }

    if (config.cursor_disabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    Window *this = malloc(sizeof(*this));
    this->config = config;
    this->window = window;

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);

    return this;
}

void window_destroy(Window *this) {
    if (this->window != NULL) {
        glfwDestroyWindow(this->window);
        this->window = NULL;
    }

    glfwTerminate();
    glfwSetErrorCallback(NULL);
    free(this);
}

GLFWwindow *window_handle(const Window *window) { return window->window; }

WindowConfig window_config(const Window *this) { return this->config; }

f32 window_content_scale(const Window *this) {
    f32 x_scale, y_scale;
    glfwGetWindowContentScale(this->window, &x_scale, &y_scale);

    return x_scale;
}

VkExtent2D window_framebuffer_size(const Window *this) {
    i32 width, height;
    glfwGetFramebufferSize(this->window, &width, &height);
    return (VkExtent2D){width, height};
}

VkExtent2D window_size(const Window *this) {
    i32 width, height;
    glfwGetWindowSize(this->window, &width, &height);
    return (VkExtent2D){width, height};
}

const char *window_get_key_name(i32 key, i32 scancode) {
    return glfwGetKeyName(key, scancode);
}

darray(const char *) window_get_required_instance_extensions(void) {
    darray(const char *) da = darray_new(const char *);

    u32 glfw_extension_count = 0;
    glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    const char **glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (u32 i = 0; i < glfw_extension_count; i++) {
        darray_push(da, glfw_extensions[i]);
    }

    return da;
}

f64 window_get_time(void) { return glfwGetTime(); }

void window_close(Window *this) {
    glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

b8 window_is_minimized(const Window *this) {
    VkExtent2D size = window_framebuffer_size(this);
    return size.height == 0 && size.width == 0;
}

void window_run(Window *this) {
    glfwSetTime(0.0);

    while (!glfwWindowShouldClose(this->window)) {
        glfwPollEvents();

        if (this->draw_frame != NULL) {
            this->draw_frame(this->context);
        }
    }
}

void window_wait_for_events(void) { glfwWaitEvents(); }

void window_set_context(Window *window, void *ctx) { window->context = ctx; }

void window_set_draw_frame(Window *window, void (*draw_frame)(void *ctx)) {
    window->draw_frame = draw_frame;
}

void window_set_on_key(Window *window,
                       void (*on_key)(void *context,
                                      i32 key,
                                      i32 scancode,
                                      i32 action,
                                      i32 mods)) {
    window->on_key = on_key;
}

void window_set_on_cursor_position(Window *window,
                                   void (*on_cursor_position)(void *ctx,
                                                              f64 x_pos,
                                                              f64 y_pos)) {
    window->on_cursor_position = on_cursor_position;
}

void window_set_on_mouse_button(
    Window *window,
    void (*on_mouse_button)(void *ctx, i32 button, i32 action, i32 mods)) {
    window->on_mouse_button = on_mouse_button;
}

void window_set_on_scroll(Window *window,
                          void (*on_scroll)(void *ctx,
                                            f64 x_offset,
                                            f64 y_offset)) {
    window->on_scroll = on_scroll;
}

// se_result window_surface_create(const struct window *window,
//                                 const struct instance *instance,
//                                 struct surface *surface) {
//     VkXlibSurfaceCreateInfoKHR create_info = {
//         .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
//         .dpy = window->display,
//         .window = window->handle,
//     };
//
//     if (vkCreateXlibSurfaceKHR(instance->handle,
//                                &create_info,
//                                NULL,
//                                &surface->handle) != VK_SUCCESS) {
//         return SE_RESULT_VULKAN_ERROR;
//     }
//
//     return SE_RESULT_OK;
// }
