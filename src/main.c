#ifdef _WIN32
#include <Windows.h>
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define KEY_ESCAPE 9
#endif

#include <stdio.h>
#include <string.h>

#ifdef _WIN32

LRESULT WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    WNDCLASSA class = {0, WinProc, 0, 0, hInstance, NULL, NULL, NULL, NULL, "GameClass"};

    RegisterClassA(&class);

    HWND windowHandle = CreateWindowA("GameClass",
                                      "Game",
                                      WS_CAPTION | WS_POPUP | WS_SYSMENU,
                                      0,
                                      0,
                                      720,
                                      480,
                                      NULL,
                                      NULL,
                                      hInstance,
                                      NULL);

    ShowWindow(windowHandle, nCmdShow);

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
#endif
