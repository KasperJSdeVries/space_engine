#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "types.h"

b8 device_create(const struct instance *instance, struct device *device);
void device_destroy(const struct instance *instance, struct device *device);

#endif // RENDER_DEVICE_H
