#ifndef FENCE_H
#define FENCE_H

#include "renderer/device.h"

typedef struct {
    const Device *device;
    VkFence handle;
} Fence;

Fence fence_new(const Device *device, b8 signaled);
Fence fence_move(Fence *other);
void fence_destroy(Fence *self);

void fence_reset(Fence *self);
void fence_wait(const Fence *self, u64 timeout);

#endif // FENCE_H
