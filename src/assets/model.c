#include "model.h"

#include "assets/material.h"
#include "assets/parsers/obj_parser.h"
#include "assets/vertex.h"
#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"

Model model_load(const char *filename) {
    parse_obj(filename);

    TODO("add model loading");

    return (Model){0};
}

Model create_cornell_box(const f32 scale) {
    UNUSED(scale);

    TODO("add cornel box");

    return (Model){0};
}

Model create_box(vec3s p0, vec3s p1, Material material) {
    Model self = {
        .vertices = darray_new(Vertex),
        .indices = darray_new(u32),
        .materials = darray_new(Material),
    };

    Vertex vertices[] = {
        {(vec3s){{p0.x, p0.y, p0.z}}, (vec3s){{-1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p0.y, p1.z}}, (vec3s){{-1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p1.z}}, (vec3s){{-1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p0.z}}, (vec3s){{-1, 0, 0}}, (vec2s){{0, 0}}, 0},

        {(vec3s){{p1.x, p0.y, p1.z}}, (vec3s){{1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p0.y, p0.z}}, (vec3s){{1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p1.y, p0.z}}, (vec3s){{1, 0, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p1.y, p1.z}}, (vec3s){{1, 0, 0}}, (vec2s){{0, 0}}, 0},

        {(vec3s){{p1.x, p0.y, p0.z}}, (vec3s){{0, 0, -1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p0.y, p0.z}}, (vec3s){{0, 0, -1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p0.z}}, (vec3s){{0, 0, -1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p1.y, p0.z}}, (vec3s){{0, 0, -1}}, (vec2s){{0, 0}}, 0},

        {(vec3s){{p0.x, p0.y, p1.z}}, (vec3s){{0, 0, 1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p0.y, p1.z}}, (vec3s){{0, 0, 1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p1.y, p1.z}}, (vec3s){{0, 0, 1}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p1.z}}, (vec3s){{0, 0, 1}}, (vec2s){{0, 0}}, 0},

        {(vec3s){{p0.x, p0.y, p0.z}}, (vec3s){{0, -1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p0.y, p0.z}}, (vec3s){{0, -1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p0.y, p1.z}}, (vec3s){{0, -1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p0.y, p1.z}}, (vec3s){{0, -1, 0}}, (vec2s){{0, 0}}, 0},

        {(vec3s){{p1.x, p1.y, p0.z}}, (vec3s){{0, 1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p0.z}}, (vec3s){{0, 1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p0.x, p1.y, p1.z}}, (vec3s){{0, 1, 0}}, (vec2s){{0, 0}}, 0},
        {(vec3s){{p1.x, p1.y, p1.z}}, (vec3s){{0, 1, 0}}, (vec2s){{0, 0}}, 0},
    };

    for (u32 i = 0; i < ARRAY_SIZE(vertices); i++) {
        darray_push(self.vertices, vertices[i]);
    }

    u32 indices[] = {0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,
                     8,  9,  10, 8,  10, 11, 12, 13, 14, 12, 14, 15,
                     16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

    for (u32 i = 0; i < ARRAY_SIZE(indices); i++) {
        darray_push(self.indices, indices[i]);
    }

    darray_push(self.materials, material);

    return self;
}

Model create_sphere(vec3s center, f32 radius, Material material) {
    Model self = {
        .vertices = darray_new(Vertex),
        .indices = darray_new(u32),
        .materials = darray_new(Material),
    };

    const u32 slices = 32;
    const u32 stacks = 16;

    const f32 pi = GLM_PI;

    for (u32 j = 0; j <= stacks; ++j) {
        const f32 j0 = pi * j / stacks;

        // Vertex
        const f32 v = radius * -sinf(j0);
        const f32 z = radius * cosf(j0);

        // Normals
        const f32 n0 = -sinf(j0);
        const f32 n1 = cosf(j0);

        for (u32 i = 0; i <= slices; ++i) {
            const float i0 = 2 * pi * i / slices;

            Vertex vertex = {
                .position = {{center.x + v * sinf(i0),
                              center.y + z,
                              center.z + v * cosf(i0)}},
                .normal = {{n0 * sinf(i0), n1, n0 * cosf(i0)}},
                .tex_coord = {{(f32)(i) / slices, (f32)(j) / stacks}},
            };

            darray_push(self.vertices, vertex);
        }
    }

    for (u32 j = 0; j < stacks; ++j) {
        for (u32 i = 0; i < slices; ++i) {
            const u32 j0 = (j + 0) * (slices + 1);
            const u32 j1 = (j + 1) * (slices + 1);
            const u32 i0 = i + 0;
            const u32 i1 = i + 1;

            darray_push(self.indices, j0 + i0);
            darray_push(self.indices, j1 + i0);
            darray_push(self.indices, j1 + i1);

            darray_push(self.indices, j0 + i0);
            darray_push(self.indices, j1 + i1);
            darray_push(self.indices, j0 + i1);
        }
    }

    darray_push(self.materials, material);

    return self;
}

void model_set_material(Model *self, Material material) {
    if (darray_length(self->materials) != 1) {
        LOG_FATAL("cannot change material on a multi-material model");
        exit(EXIT_FAILURE);
    }

    self->materials[0] = material;
}

void model_transform(Model *self, mat4s transform) {
    mat4s transformIT = mat4_transpose(mat4_inv(transform));

    for (u32 i = 0; i < darray_length(self->vertices); i++) {
        Vertex *vertex = &self->vertices[i];

        vertex->position = mat4_mulv3(transform, vertex->position, 1.0);
        vertex->normal = mat4_mulv3(transformIT, vertex->normal, 0.0);
    }
}
