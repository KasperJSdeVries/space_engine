#ifndef GAME_H
#define GAME_H

#include "core/defines.h"
#include "math/vec3.h"
#include "renderer/types.h"

struct player {
    vec3 position;
    vec3 look_direction;
};

struct ground_plane {
    struct renderbuffer vertices;
    struct renderbuffer indices;
	u32 index_count;
	struct pipeline pipeline;
};

struct world {
    struct player player;
    struct ground_plane ground_plane;
};

void world_setup(struct world *world, const struct renderer *renderer);
void world_update(struct world *world, const struct renderer *renderer, f32 delta_time);
void world_cleanup(struct world *world, const struct renderer *renderer);

#endif // GAME_H
