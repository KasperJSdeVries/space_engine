#include "fence.h"

Fence fence_new(const Device *device, b8 signaled) {
    Fence self = {
        .device = device,
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0,
    };

    vulkan_check(vkCreateFence(device->handle, &fence_info, NULL, &self.handle),
                 "create fence");

    return self;
}

Fence fence_move(Fence *other) {
    Fence self = {
        .handle = other->handle,
        .device = other->device,
    };

    other->handle = NULL;

    return self;
}

void fence_destroy(Fence *self) {
    if (self->handle != NULL) {
        vkDestroyFence(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}

void fence_reset(Fence *self) {
    vulkan_check(vkResetFences(self->device->handle, 1, &self->handle),
                 "reset fence");
}

void fence_wait(const Fence *self, u64 timeout) {
    vulkan_check(vkWaitForFences(self->device->handle,
                                 1,
                                 &self->handle,
                                 VK_TRUE,
                                 timeout),
                 "wait for fence");
}
