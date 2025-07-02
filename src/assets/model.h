#ifndef MODEL_H
#define MODEL_H

#include "assets/material.h"
#include "assets/vertex.h"
#include "containers/darray.h"

typedef struct {
    darray(Vertex) vertices;
    darray(u32) indices;
    darray(Material) materials;
} Model;

Model model_load(const char *filename);
Model create_cornell_box(const f32 scale);
Model create_box(vec3s p0, vec3s p1, const Material *material);
Model create_sphere(vec3s center, f32 radius, const Material *material);

void model_set_material(Model *self, const Material *material);
void model_transform(Model *self, mat4s transform);

#endif // MODEL_H
