#ifndef VERTEX_H
#define VERTEX_H

#include "cglm/types-struct.h"
#include "core/assert.h"
#include "core/defines.h"
#include "renderer/vulkan.h"

#include <string.h>

typedef struct {
    vec3s position;
    vec3s normal;
    vec2s tex_coord;
    i32 material_index;
} Vertex;

static inline VkVertexInputBindingDescription vertex_binding_description(void) {
    return (VkVertexInputBindingDescription){
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

static inline void vertex_attribute_descriptions(
    u32 *attribute_count,
    VkVertexInputAttributeDescription *out_attribute_descriptions) {

    VkVertexInputAttributeDescription attribute_descriptions[] = {
        (VkVertexInputAttributeDescription){
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        },
        (VkVertexInputAttributeDescription){
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal),
        },
        (VkVertexInputAttributeDescription){
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, tex_coord),
        },
        (VkVertexInputAttributeDescription){
            .binding = 0,
            .location = 3,
            .format = VK_FORMAT_R32_SINT,
            .offset = offsetof(Vertex, material_index),
        },
    };

    if (out_attribute_descriptions == NULL) {
        if (attribute_count != NULL) {
            *attribute_count = ARRAY_SIZE(attribute_descriptions);
        }
        return;
    }

    ASSERT(*attribute_count == ARRAY_SIZE(attribute_descriptions));

    memcpy(out_attribute_descriptions,
           attribute_descriptions,
           sizeof(attribute_descriptions));
}

#endif // VERTEX_H
