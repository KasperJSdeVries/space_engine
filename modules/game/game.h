#ifndef GAME_H
#define GAME_H

#include "core/defines.h"
#include "math/vec3.h"

struct player {
	vec3 position;
	vec3 look_direction;
};

struct ground_plane {
	struct render_buffer *model;
};

struct world {
	struct player player;
	struct ground_plane ground_plane;
};

void world_setup(struct world *world);
void world_update(struct world *world, f32 delta_time);
void world_cleanup(struct world *world);

#endif // GAME_H
