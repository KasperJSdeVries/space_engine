#ifndef CORE_RESULT_H
#define CORE_RESULT_H

#include "logging.h"

#define ENUMERATE_RESULTS                                                      \
    __ENUMERATE_RESULT(OK, "Ok")                                               \
    __ENUMERATE_RESULT(WINDOW_ERROR, "Window Error")                           \
    __ENUMERATE_RESULT(WINDOW_CLOSED, "Window Closed")                         \
    __ENUMERATE_RESULT(WINDOW_RESIZED, "Window Resized")                       \
    __ENUMERATE_RESULT(VULKAN_ERROR, "Vulkan Error")

typedef enum {
#define __ENUMERATE_RESULT(name, _) SE_RESULT_##name,
    ENUMERATE_RESULTS
#undef __ENUMERATE_RESULT
} se_result;

static inline char *se_result_get_name(se_result result) {
    switch (result) {
#define __ENUMERATE_RESULT(name, _)                                            \
case SE_RESULT_##name:                                                         \
    return #name;
        ENUMERATE_RESULTS
#undef __ENUMERATE_RESULT
    }
    UNREACHABLE("");
}

static inline char *se_result_get_formatted_name(se_result result) {
    switch (result) {
#define __ENUMERATE_RESULT(name, formatted_name)                               \
case SE_RESULT_##name:                                                         \
    return formatted_name;
        ENUMERATE_RESULTS
#undef __ENUMERATE_RESULT
    }
    UNREACHABLE("");
}

#endif // CORE_RESULT_H
