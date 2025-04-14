#include "game.h"

#include "core/defines.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "renderer/buffer.h"
#include "renderer/pipeline.h"
#include "renderer/types.h"
#include "vulkan/vulkan_core.h"

#include <string.h>

static void generate_ground_plane(const struct renderer *renderer,
                                  f32 side_length,
                                  u32 resolution,
                                  struct ground_plane *ground_plane);
static void ground_plane_cleanup(const struct renderer *renderer,
                                 struct ground_plane *ground_plane);
static void ground_plane_render(const struct renderer *renderer,
                                struct ground_plane *ground_plane);

void world_setup(struct world *world, const struct renderer *renderer) {
    generate_ground_plane(renderer, 10.0f, 50, &world->ground_plane);
}

void world_update(struct world *world,
                  const struct renderer *renderer,
                  f32 delta_time) {
    UNUSED(delta_time);
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
    u32 vertex_count = resolution * resolution;
    u32 index_count = (resolution - 1) * (resolution - 1) * 6;
    ground_plane->index_count = index_count;

    vec3 vertices[vertex_count];
    u32 indices[index_count];

    u32 current_vertex = 0;
    for (u32 y = 0; y < resolution; y++) {
        for (u32 x = 0; x < resolution; x++) {
            u32 i = x + y * resolution;
            vec2 percent = vec2_divs((vec2){{x, y}}, resolution - 1);
            vec3 local_up = {{0.0f, -1.0f, 0.0f}};
            vec3 axis_a = {{local_up.y, local_up.z, local_up.x}};
            vec3 axis_b = vec3_cross(local_up, axis_a);
            vec3 vertex =
                vec3_add(vec3_scale(axis_a, (percent.x - 0.5f) * 2.0f),
                         vec3_scale(axis_b, (percent.y - 0.5f) * 2.0f));
            vertices[i] =
                vec3_add(start_point, vec3_scale(vertex, side_length));

            if (x != resolution - 1 && y != resolution - 1) {
                indices[current_vertex] = i;
                indices[current_vertex + 1] = i + resolution + 1;
                indices[current_vertex + 2] = i + resolution;

                indices[current_vertex + 3] = i;
                indices[current_vertex + 4] = i + 1;
                indices[current_vertex + 5] = i + resolution + 1;
                current_vertex += 6;
            }
        }
    }

    struct renderbuffer vertex_staging_buffer;
    if (!render_buffer_create(&renderer->device,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                              sizeof(vertices),
                              &vertex_staging_buffer)) {
        return;
    }

    void *vertex_dest;
    render_buffer_map_memory(&renderer->device,
                             &vertex_staging_buffer,
                             &vertex_dest);
    memcpy(vertex_dest, vertices, sizeof(vertices));
    render_buffer_unmap_memory(&renderer->device, &vertex_staging_buffer);

    if (!render_buffer_create(&renderer->device,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              sizeof(vertices),
                              &ground_plane->vertices)) {
        return;
    }

    render_buffer_copy(renderer,
                       &vertex_staging_buffer,
                       &ground_plane->vertices);
    render_buffer_destroy(&renderer->device, &vertex_staging_buffer);

    struct renderbuffer index_staging_buffer;
    if (!render_buffer_create(&renderer->device,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                              sizeof(indices),
                              &index_staging_buffer)) {
        return;
    }
    void *index_dest;
    render_buffer_map_memory(&renderer->device,
                             &index_staging_buffer,
                             &index_dest);
    memcpy(index_dest, indices, sizeof(indices));
    render_buffer_unmap_memory(&renderer->device, &index_staging_buffer);

    if (!render_buffer_create(&renderer->device,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              sizeof(indices),
                              &ground_plane->indices)) {
        return;
    }

    render_buffer_copy(renderer, &index_staging_buffer, &ground_plane->indices);
    render_buffer_destroy(&renderer->device, &index_staging_buffer);

    struct pipeline_builder pipeline_builder = pipeline_builder_new(renderer);
    pipeline_builder_set_shaders(&pipeline_builder,
                                 "shaders/triangle.vert.spv",
                                 "shaders/triangle.frag.spv");
    pipeline_builder_add_input_binding(&pipeline_builder,
                                       0,
                                       sizeof(vec3),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    pipeline_builder_add_input_attribute(&pipeline_builder,
                                         0,
                                         0,
                                         VK_FORMAT_R32G32B32_SFLOAT,
                                         0);
    pipeline_builder_set_ubo_size(&pipeline_builder,
                                  sizeof(struct uniform_buffer_object));
    pipeline_builder_build(&pipeline_builder,
                           renderer->renderpass.handle,
                           &ground_plane->pipeline);
}

static void ground_plane_cleanup(const struct renderer *renderer,
                                 struct ground_plane *ground_plane) {
    render_buffer_destroy(&renderer->device, &ground_plane->indices);
    render_buffer_destroy(&renderer->device, &ground_plane->vertices);
    pipeline_destroy(&renderer->device, &ground_plane->pipeline);
}

static void ground_plane_render(const struct renderer *renderer,
                                struct ground_plane *ground_plane) {

    struct uniform_buffer_object ubo = {
        .model =
            rotate(MAT4_IDENTITY, DEG2RAD(0.0f), (vec3){{0.0f, 0.0f, 1.0f}}),
        .view = lookat((vec3){{5.0, 5.0, 5.0}},
                       (vec3){{0.0, 0.0, 0.0}},
                       (vec3){{0.0, -1.0, 0.0}}),
        .projection = perspective(DEG2RAD(45.0f),
                                  renderer->swapchain.extent.width /
                                      (f32)renderer->swapchain.extent.height,
                                  0.1f,
                                  1000.0f),
    };

    memcpy(ground_plane->pipeline.uniform_buffer_mapped, &ubo, sizeof(ubo));
    pipeline_bind(renderer, &ground_plane->pipeline);

    VkBuffer vertex_buffers[] = {ground_plane->vertices.handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(
        renderer->commandbuffers[renderer->current_frame].handle,
        0,
        1,
        vertex_buffers,
        offsets);

    vkCmdBindIndexBuffer(
        renderer->commandbuffers[renderer->current_frame].handle,
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
