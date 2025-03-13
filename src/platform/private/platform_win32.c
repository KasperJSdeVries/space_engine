#include "platform.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct se_platform_state {
    HINSTANCE h_instance;
} se_platform_state;

static se_platform_state *state_ptr;

struct se_window_platform_state {
    HWND handle;
};

HMODULE GetCurrentModuleHandle(void);
LRESULT CALLBACK win32_process_message(HWND hwnd, uint32_t msg, WPARAM w_param, LPARAM l_param);

bool platform_system_startup(size_t *memory_requirement, se_platform_state *state) {
    *memory_requirement = sizeof(se_platform_state);
    if (state == NULL) {
        return true;
    }
    state_ptr = state;
    state_ptr->h_instance = GetModuleHandleW(0);

    HICON icon = LoadIconW(state_ptr->h_instance, IDI_APPLICATION);
    WNDCLASSEXW wc = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_DBLCLKS,
        .lpfnWndProc = win32_process_message,
        .hInstance = state_ptr->h_instance,
        .hIcon = icon,
        .hIconSm = icon,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = L"space_engine_window_class",
    };

    if (!RegisterClassExW(&wc)) {
        DWORD last_error = GetLastError();
        LPWSTR wmessage_buf = 0;

        (void)FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             last_error,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             (LPWSTR)&wmessage_buf,
                             0,
                             NULL);

        MessageBoxW(NULL, wmessage_buf, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        LocalFree(wmessage_buf);

        return false;
    }

    return true;
}

bool platform_window_create(struct se_window_config *config, struct se_window *window) {
    if (!window) {
        return false;
    }

    int32_t window_x = config->position_x;
    int32_t window_y = config->position_y;
    int32_t window_width = config->width;
    int32_t window_height = config->height;

    uint32_t window_style =
        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    uint32_t window_ex_style = WS_EX_APPWINDOW;

    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    window_x += border_rect.left;
    window_y += border_rect.top;

    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    if (config->title) {
        window->title = _strdup(config->title);
    } else {
        window->title = _strdup("Space Engine Window");
    }

    window->platform_state = malloc(sizeof(struct se_window_platform_state));

    WCHAR wtitle[256];
    (void)MultiByteToWideChar(CP_UTF8, 0, window->title, -1, wtitle, 256);
    window->platform_state->handle = CreateWindowExW(window_ex_style,
                                                     L"space_engine_window_class",
                                                     wtitle,
                                                     window_style,
                                                     window_x,
                                                     window_y,
                                                     window_width,
                                                     window_height,
                                                     0,
                                                     0,
                                                     state_ptr->h_instance,
                                                     0);

    if (window->platform_state->handle == 0) {
        DWORD last_error = GetLastError();
        LPWSTR wmessage_buf = 0;

        (void)FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             last_error,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             (LPWSTR)&wmessage_buf,
                             0,
                             NULL);

        MessageBoxW(NULL, wmessage_buf, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        LocalFree(wmessage_buf);

        return false;
    }

    ShowWindow(window->platform_state->handle, SW_NORMAL);

    return true;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, uint32_t msg, WPARAM w_param, LPARAM l_param) {
    return DefWindowProcW(hwnd, msg, w_param, l_param);
}

HMODULE GetCurrentModuleHandle(void) {
    HMODULE ImageBase;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)&GetCurrentModuleHandle,
                           &ImageBase)) {
        return ImageBase;
    }
    return 0;
}

#endif // _WIN32
