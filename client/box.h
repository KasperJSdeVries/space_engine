#ifndef BOX_H
#define BOX_H

#include "renderer/types.h"

struct model {
    struct renderbuffer vertices;
    struct renderbuffer indices;
    struct pipeline pipeline;
    mat4s matrix;
};

struct box {
    vec3s position;
    vec3s rotation;
    vec3s scale;
    struct model model;
};

struct box box_new(const struct renderer *renderer);
void box_destroy(const struct renderer *renderer, struct box *box);
void box_set_position(struct box *box, vec3s position);
void box_set_rotation(struct box *box, vec3s rotation);
void box_set_scale(struct box *box, vec3s scale);
void box_render(const struct renderer *renderer, const struct box *box);

#endif // BOX_H
