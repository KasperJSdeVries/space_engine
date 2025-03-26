#include "game.h"

#include "core/logging.h"
#include "math/vec3.h"
#include "render_system/buffer.h"
#include "render_system/pipeline.h"
#include "render_system/types.h"

#include <string.h>

static void generate_ground_plane(const struct renderer *renderer,
                                  f32 side_length,
                                  u32 resolution,
                                  struct ground_plane *ground_plane);
static void ground_plane_cleanup(const struct renderer *renderer,
                                 struct ground_plane *ground_plane);
static void ground_plane_render(const struct renderer *renderer, struct ground_plane *ground_plane);

void world_setup(struct world *world, const struct renderer *renderer) {
    generate_ground_plane(renderer, 10.0f, 10, &world->ground_plane);
}

void world_update(struct world *world, const struct renderer *renderer, f32 delta_time) {
    ground_plane_render(renderer, &world->ground_plane);
}

void world_cleanup(struct world *world, const struct renderer *renderer) {
    ground_plane_cleanup(renderer, &world->ground_plane);
}

static void generate_ground_plane(const struct renderer *renderer,
                                  f32 side_length,
                                  u32 resolution,
                                  struct ground_plane *ground_plane) {
    vec3 start_point = {{-side_length / 2.0f, 0, -side_length / 2.0f}};
    f32 step_size = side_length / (f32)(resolution - 1);
    u32 vertex_count = resolution * resolution;
    u32 index_count = (resolution - 1) * (resolution - 1) * 6;
    ground_plane->index_count = index_count;

    vec3 vertices[vertex_count];
    u32 indices[index_count];

    u32 current_vertex = 0;
    for (u32 y = 0; y < resolution; y++) {
        for (u32 x = 0; x < resolution; x++) {
            vertices[current_vertex] =
                vec3_add(start_point, (vec3){{x * step_size, 0, y * step_size}});
            LOG_INFO("(%d, %d): " VEC3_FORMAT, x, y, VEC3_PRINT(vertices[current_vertex]));
        }
    }

    if (!render_buffer_create(&renderer->device,
                              RENDERBUFFER_USAGE_VERTEX,
                              sizeof(vertices),
                              &ground_plane->vertices)) {
        return;
    }
    void *vertices_ptr = render_buffer_map_memory(&renderer->device, &ground_plane->vertices);
    memcpy(vertices_ptr, vertices, sizeof(vertices));
    render_buffer_unmap_memory(&renderer->device, &ground_plane->vertices);

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

    if (!render_buffer_create(&renderer->device,
                              RENDERBUFFER_USAGE_INDEX,
                              sizeof(indices),
                              &ground_plane->indices)) {
        return;
    }
    void *indices_ptr = render_buffer_map_memory(&renderer->device, &ground_plane->indices);
    memcpy(indices_ptr, indices, sizeof(indices));
    render_buffer_unmap_memory(&renderer->device, &ground_plane->indices);

    struct graphics_pipeline_config pipeline_config = {
        .vertex_shader_path = "shaders/triangle.vert.spv",
        .fragment_shader_path = "shaders/triangle.frag.spv",
        .vertex_size = sizeof(vec3),
    };

    graphics_pipeline_create(renderer, pipeline_config, &ground_plane->pipeline);
}

static void ground_plane_cleanup(const struct renderer *renderer,
                                 struct ground_plane *ground_plane) {
    pipeline_destroy(&renderer->device, &ground_plane->pipeline);
    render_buffer_destroy(&renderer->device, &ground_plane->indices);
    render_buffer_destroy(&renderer->device, &ground_plane->vertices);
}

static void ground_plane_render(const struct renderer *renderer,
                                struct ground_plane *ground_plane) {
    pipeline_bind(renderer, &ground_plane->pipeline);

    VkBuffer vertex_buffers[] = {ground_plane->vertices.handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(renderer->commandbuffers[renderer->current_frame].handle,
                           0,
                           1,
                           vertex_buffers,
                           offsets);

    vkCmdBindIndexBuffer(renderer->commandbuffers[renderer->current_frame].handle,
                         ground_plane->indices.handle,
                         0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(renderer->commandbuffers[renderer->current_frame].handle,
                     ground_plane->index_count,
                     1,
                     0,
                     0,
                     0);
}
