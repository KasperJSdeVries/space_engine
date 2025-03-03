#include "core/assert.h"
#include "core/defines.h"
#include "platform/platform.h"
#include "render_system/render.h"
#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    u64 platform_state_size = 0;
    platform_system_startup(&platform_state_size, NULL);
    struct se_platform_state *platform_state = malloc(platform_state_size);
    platform_system_startup(&platform_state_size, platform_state);

    struct se_window_config window_config = {
        .width = 720,
        .height = 480,
    };
    struct se_window window;
    platform_window_create(&window_config, &window);

    ASSERT(render_system_startup());

    for (b8 quit = false; quit == false;) {
        if (!platform_system_poll(platform_state)) {
            quit = true;
        }
    }

    render_system_shutdown();

    platform_window_destroy(&window);

    platform_system_shutdown(platform_state);
    free(platform_state);
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
#endif
