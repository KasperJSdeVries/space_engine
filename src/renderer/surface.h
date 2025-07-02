#ifndef SURFACE_H
#define SURFACE_H

#include "renderer/instance.h"

typedef struct {
    VkSurfaceKHR handle;
    const Instance *instance;
} Surface;

Surface surface_new(const Instance *instance);
void surface_destroy(Surface *surface);

#endif // SURFACE_H
