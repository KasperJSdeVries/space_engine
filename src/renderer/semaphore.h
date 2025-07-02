#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "renderer/device.h"

typedef struct {
    const Device *device;
    VkSemaphore handle;
} Semaphore;

Semaphore semaphore_new(const Device *device);
Semaphore semaphore_move(Semaphore *other);
void semaphore_destroy(Semaphore *self);

#endif // SEMAPHORE_H
