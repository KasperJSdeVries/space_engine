#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H

#include "types.h"

struct graphics_pipeline_config {
    const char *vertex_shader_path;
    const char *fragment_shader_path;
    u32 vertex_size;
};

b8 graphics_pipeline_create(const struct renderer *renderer,
                            struct graphics_pipeline_config config,
                            struct pipeline *pipeline);
void pipeline_destroy(const struct device *device, struct pipeline *pipeline);
void pipeline_bind(const struct renderer *renderer, const struct pipeline *pipeline);

#endif // RENDER_PIPELINE_H
