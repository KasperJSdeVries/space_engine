#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H

#include "types.h"

b8 pipeline_create(const struct device *device, struct pipeline *pipeline);
void pipeline_destroy(const struct device *device, struct pipeline *pipeline);

#endif // RENDER_PIPELINE_H
