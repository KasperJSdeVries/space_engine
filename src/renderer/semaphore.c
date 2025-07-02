#include "semaphore.h"
#include "vulkan/vulkan_core.h"

Semaphore semaphore_new(const Device *device) {
    Semaphore self = {
        .device = device,
    };

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    vulkan_check(
        vkCreateSemaphore(device->handle, &semaphore_info, NULL, &self.handle),
        "create semaphore");

    return self;
}

Semaphore semaphore_move(Semaphore *other) {
    Semaphore self = {
        .device = other->device,
        .handle = other->handle,
    };

    other->handle = NULL;

    return self;
}

void semaphore_destroy(Semaphore *self) {
    if (self->handle != NULL) {
        vkDestroySemaphore(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}
