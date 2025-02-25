#include "platform.h"

#ifdef __linux__

#include <X11/X.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct se_platform_state {
    Display *display;
} se_platform_state;

static se_platform_state *state_ptr;

struct se_window_platform_state {
    Window window;
};

bool platform_system_startup(size_t *memory_requirement, se_platform_state *state) {
    *memory_requirement = sizeof(se_platform_state);
    if (state == NULL) {
        return true;
    }

    state_ptr = state;
    state_ptr->display = XOpenDisplay(NULL);

    if (!state_ptr->display) {
        fprintf(stderr, "error: can't open connection to display server.\n");
        return false;
    }

    return true;
}

bool platform_window_create(struct se_window_config *config, struct se_window *window) {
    if (!window) {
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

    long event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                      ExposureMask | PointerMotionMask | StructureNotifyMask;
    XSelectInput(state_ptr->display, window->platform_state->window, event_mask);

    // TODO: add window-manager interaction
    /*Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);*/
    /*XSetWMProtocols(display, window, &wm_delete_window, 1);*/

    XMapWindow(state_ptr->display, window->platform_state->window);

    return true;
}

#endif // __linux__
