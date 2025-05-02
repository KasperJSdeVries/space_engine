#include "box.h"
#include "cglm/struct/euler.h"
#include "core/defines.h"
#include "renderer/buffer.h"
#include "renderer/pipeline.h"
#include "renderer/types.h"
#include "vulkan/vulkan_core.h"

#include <string.h>

static void box_update_model(struct box *box);

struct box box_new(const struct renderer *renderer) {
    struct box box = {
        .position = vec3_zero(),
        .rotation = vec3_zero(),
        .scale = vec3_one(),
    };

    box_update_model(&box);

    vec3s vertices[8] = {
        {{-0.5, -0.5, -0.5}},
        {{0.5, -0.5, -0.5}},
        {{0.5, 0.5, -0.5}},
        {{-0.5, 0.5, -0.5}},
        {{-0.5, -0.5, 0.5}},
        {{0.5, -0.5, 0.5}},
        {{0.5, 0.5, 0.5}},
        {{-0.5, 0.5, 0.5}},
    };

    u32 indices[6 * 6] = {0, 1, 3, 3, 1, 2, 1, 5, 2, 2, 5, 6, 5, 4, 6, 6, 4, 7,
                          4, 0, 7, 7, 0, 3, 3, 2, 7, 7, 2, 6, 4, 5, 0, 0, 5, 1};

    struct renderbuffer vertex_staging_buffer;
    render_buffer_create(&renderer->device,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                         sizeof(vertices),
                         &vertex_staging_buffer);

    void *vertex_dest;
    render_buffer_map_memory(&renderer->device,
                             &vertex_staging_buffer,
                             &vertex_dest);
    memcpy(vertex_dest, vertices, sizeof(vertices));
    render_buffer_unmap_memory(&renderer->device, &vertex_staging_buffer);

    render_buffer_create(&renderer->device,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         sizeof(vertices),
                         &box.model.vertices);

    render_buffer_copy(renderer, &vertex_staging_buffer, &box.model.vertices);
    render_buffer_destroy(&renderer->device, &vertex_staging_buffer);

    struct renderbuffer index_staging_buffer;
    render_buffer_create(&renderer->device,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                         sizeof(indices),
                         &index_staging_buffer);
    void *index_dest;
    render_buffer_map_memory(&renderer->device,
                             &index_staging_buffer,
                             &index_dest);
    memcpy(index_dest, indices, sizeof(indices));
    render_buffer_unmap_memory(&renderer->device, &index_staging_buffer);

    render_buffer_create(&renderer->device,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         sizeof(indices),
                         &box.model.indices);

    render_buffer_copy(renderer, &index_staging_buffer, &box.model.indices);
    render_buffer_destroy(&renderer->device, &index_staging_buffer);

    struct pipeline_builder builder = pipeline_builder_new(renderer);

    pipeline_builder_set_shaders(&builder,
                                 "shaders/triangle.vert.spv",
                                 "shaders/triangle.frag.spv");
    pipeline_builder_add_input_binding(&builder,
                                       0,
                                       sizeof(vec3),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    pipeline_builder_add_input_attribute(&builder,
                                         0,
                                         0,
                                         VK_FORMAT_R32G32B32_SFLOAT,
                                         0);
    pipeline_builder_set_ubo_size(&builder,
                                  sizeof(struct uniform_buffer_object));
    pipeline_builder_build(&builder,
                           renderer->renderpass.handle,
                           &box.model.pipeline);

    return box;
}

void box_destroy(const struct renderer *renderer, struct box *box) {
    render_buffer_destroy(&renderer->device, &box->model.vertices);
    render_buffer_destroy(&renderer->device, &box->model.indices);
    pipeline_destroy(&renderer->device, &box->model.pipeline);
}

void box_set_position(struct box *box, vec3s position) {
    box->position = position;
    box_update_model(box);
}

void box_set_rotation(struct box *box, vec3s rotation) {
    box->rotation = rotation;
    box_update_model(box);
}

void box_set_scale(struct box *box, vec3s scale) {
    box->scale = scale;
    box_update_model(box);
}

void box_render(const struct renderer *renderer, const struct box *box) {
    struct uniform_buffer_object ubo = {
        .model = box->model.matrix,
        .view = glms_lookat((vec3s){{5.0, 5.0, 5.0}},
                            (vec3s){{0.0, 0.0, 0.0}},
                            (vec3s){{0.0, -1.0, 0.0}}),
        .projection =
            glms_perspective(glm_rad(50.625f),
                             renderer->swapchain.extent.width /
                                 (f32)renderer->swapchain.extent.height,
                             0.1f,
                             1000.0f),
    };

    memcpy(box->model.pipeline.uniform_buffer_mapped, &ubo, sizeof(ubo));
    pipeline_bind(renderer, &box->model.pipeline);

    VkBuffer vertex_buffers[] = {box->model.vertices.handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(
        renderer->commandbuffers[renderer->current_frame].handle,
        0,
        1,
        vertex_buffers,
        offsets);

    vkCmdBindIndexBuffer(
        renderer->commandbuffers[renderer->current_frame].handle,
        box->model.indices.handle,
        0,
        VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(renderer->commandbuffers[renderer->current_frame].handle,
                     36,
                     1,
                     0,
                     0,
                     0);
}

static void box_update_model(struct box *box) {
    mat4s translation = glms_translate_make(box->position);
    mat4s rotation = glms_euler_xyz(box->rotation);
    mat4s scaling = glms_scale_make(box->scale);

    box->model.matrix = mat4_mul(mat4_mul(translation, rotation), scaling);
}
