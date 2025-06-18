#ifndef SYSTEM_H
#define SYSTEM_H

#include "core/defines.h"

#define MAX_REQUIRED_COMPONENTS 8

typedef enum {
    // TODO: add more system flags
    SYSTEM_FLAG_NETWORKED = 1 << 0,
} SystemFlag;

typedef void (*system_run)(void **components, float dt);

typedef struct {
    const char *name;
    u32 required_components[MAX_REQUIRED_COMPONENTS];
    u64 required_count;
    SystemFlag flags;
} System;

#endif // SYSTEM_H
