#include "../platform/platform.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define KEY_ESCAPE 9
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    size_t platform_state_size = 0;
    platform_system_startup(&platform_state_size, NULL);
    struct se_platform_state *platform_state = malloc(platform_state_size);
    platform_system_startup(&platform_state_size, platform_state);

    struct se_window_config window_config = {0};
    struct se_window window;
    platform_window_create(&window_config, &window);
}

#ifdef _WIN32

int windows_main(void) {
    MSG msg;
    for (;;) {
        if (GetMessageA(&msg, NULL, 0, 0) == 0) {
            break;
        }

        switch (msg.message) {
        case WM_KEYDOWN:
            switch (msg.wParam) {
            case VK_ESCAPE:
                PostQuitMessage(0);
                continue;
            }
            break;
        }

        DispatchMessageA(&msg);
    }

    DestroyWindow(windowHandle);
    UnregisterClassA("GameClass", hInstance);

    return 0;
}
#else
int linux_main(void) {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "error: can't open connection to display server.\n");
        return 1;
    }

    Window window = XCreateSimpleWindow(display,
                                        DefaultRootWindow(display),
                                        0,
                                        0,
                                        720,
                                        480,
                                        0,
                                        BlackPixel(display, DefaultScreen(display)),
                                        BlackPixel(display, DefaultScreen(display)));

    long event_mask = ExposureMask | KeyPressMask;
    XSelectInput(display, window, event_mask);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XMapWindow(display, window);

    XEvent event;

    for (int loop = 1; loop;) {
        XNextEvent(display, &event);

        switch (event.type) {
        case Expose:
            puts("window expose");
            break;

        case KeyPress:
            printf("key: %d\n", event.xkey.keycode);
            if (event.xkey.keycode == KEY_ESCAPE) {
                loop = 0;
            }
            break;

        case ClientMessage:
            if (memcmp(event.xclient.data.b, &wm_delete_window, sizeof(wm_delete_window)) == 0) {
                loop = 0;
            }
            break;
        }
    }

    puts("window close");

    XUnmapWindow(display, window);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
#endif
