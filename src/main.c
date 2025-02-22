#include <X11/X.h>
#include <X11/Xlib.h>

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define KEY_ESCAPE 9

typedef void (*declare_system)(void);

int main(void) {
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

    void *test_so = dlopen("./libtest_component.so", RTLD_NOW);
    declare_system func = (declare_system)dlsym(test_so, "declare_system");
    func();
    dlclose(test_so);

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
