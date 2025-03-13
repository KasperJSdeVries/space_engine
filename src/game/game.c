#include "game.h"

#include "math/vec3.h"

#include <stdio.h>

static void generate_ground_plane(f32 side_length,
                                  u32 resolution,
                                  struct ground_plane *ground_plane);

void world_setup(struct world *world) { generate_ground_plane(10.0f, 10, &world->ground_plane); }

void world_update(struct world *world, f32 delta_time);

void world_cleanup(struct world *world);

static void generate_ground_plane(f32 side_length,
                                  u32 resolution,
                                  struct ground_plane *ground_plane) {
    vec3 start_point = {{-side_length / 2.0f, 0, -side_length / 2.0f}};
    f32 step_size = side_length / (f32)(resolution - 1);
    u32 vertex_count = resolution * resolution;
    u32 index_count = (resolution - 1) * (resolution - 1) * 6;

    vec3 vertices[vertex_count];
    u32 indices[index_count];

    u32 current_vertex = 0;
    for (u32 y = 0; y < resolution; y++) {
        for (u32 x = 0; x < resolution; x++) {
            vertices[current_vertex] =
                vec3_add(start_point, (vec3){{x * step_size, 0, y * step_size}});
        }
    }

    u32 current_index = 0;
    for (u32 y = 1; y < resolution; y++) {
        for (u32 x = 1; x < resolution; x++) {
            indices[current_index] = (x - 1) + (y - 1) * resolution; // 0,0
            indices[current_index + 1] = (x - 1) + (y)*resolution;   // 0,1
            indices[current_index + 2] = (x) + (y - 1) * resolution; // 1,0
            indices[current_index + 3] = (x - 1) + (y)*resolution;   // 0,1
            indices[current_index + 4] = (x) + (y)*resolution;       // 1,1
            indices[current_index + 5] = (x) + (y - 1) * resolution; // 1,0

            current_index += 6;
        }
    }
}
