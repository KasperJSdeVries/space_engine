#ifndef RENDER_INSTANCE_H
#define RENDER_INSTANCE_H

#include "types.h"

static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32 validation_layer_count = sizeof(validation_layers) / sizeof(*validation_layers);

b8 instance_create(struct instance* instance);
void instance_destroy(struct instance* instance);

#endif  // RENDER_INSTANCE_H
