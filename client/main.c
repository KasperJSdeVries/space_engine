#include "assets/material.h"
#include "assets/parsers/gltf_parser.h"
#include "core/defines.h"
#include "renderer/application.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>

static f32 randf(void) { return (f32)rand() / (f32)RAND_MAX; }

// static void print_node_tree(const Gltf gltf, u32 node_idx, u32 depth) {
//     char indent[101] = "";
//     for (u32 i = 0; i < depth && i <= 50; i++) {
//         strcat(indent, "  ");
//     }
//
//     LOG_INFO("%s|-'%s'", indent, gltf.nodes[node_idx].name);
//     if (gltf.nodes[node_idx].mesh != -1) {
//         LOG_INFO("%s  |-'%s'", indent, gltf.meshes[gltf.nodes[node_idx].mesh].name);
//     }
//     if (gltf.nodes[node_idx].camera != -1) {
//         LOG_INFO("%s  |-'%s'", indent, gltf.cameras[gltf.nodes[node_idx].camera].name);
//     }
//     if (gltf.nodes[node_idx].children) {
//         for (u32 k = 0; k < darray_length(gltf.nodes[node_idx].children); k++) {
//             print_node_tree(gltf, gltf.nodes[node_idx].children[k], depth + 1);
//         }
//     }
// }

typedef struct {
    vec3s translation;
    versors rotation;
    vec3s scale;
    u32 mesh_index;
} MeshData;

void get_meshes(const Gltf *gltf, MeshData *parent, u32 node_index, darray(MeshData) * meshes) {
    if (gltf == NULL || meshes == NULL) {
        LOG_ERROR("get_meshes: gltf or meshes is NULL");
        return;
    }
    if (node_index >= darray_length(gltf->nodes)) {
        LOG_ERROR("get_meshes: node_index out of bounds");
        return;
    }

    GltfNode *node = &(gltf->nodes[node_index]);
    LOG_INFO("Processing node %u", node_index);
    LOG_INFO("Node name: %s", node->name);

    MeshData *self = NULL;
    if (node->mesh != -1) {
        MeshData mesh = {0};

        if (parent != NULL) {
            mesh = (MeshData){
                .translation = vec3_add(parent->translation, node->translation),
                .rotation = quat_add(parent->rotation, node->rotation),
                .scale = vec3_mul(parent->scale, node->scale),
                .mesh_index = node->mesh,
            };
        } else {
            mesh = (MeshData){
                .translation = node->translation,
                .rotation = node->rotation,
                .scale = node->scale,
                .mesh_index = node->mesh,
            };
        }

        darray_push(*meshes, mesh);
        self = &((*meshes)[darray_length(*meshes) - 1]);
    }

    if (node->children) {
        for (u32 i = 0; i < darray_length(node->children); i++) {
            LOG_INFO("Child node index: %u", node->children[i]);
            get_meshes(gltf, self, node->children[i], meshes);
        }
    }
}

int main(void) {
    WindowConfig window_config = {
        .title = "Vulkan Window",
        .width = 1080,
        .height = 720,
        .cursor_disabled = true,
        .fullscreen = false,
        .resizable = true,
    };

    Gltf gltf = gltf_parse("assets/models/main_sponza/NewSponza_Main_glTF_003.gltf");

    darray(MeshData) meshes = darray_new(MeshData);

    for (u32 i = 0; i < darray_length(meshes); i++) {
        glms_vec3_print(meshes[i].translation, stdout);
        glm_vec4_print(meshes[i].rotation.raw, stdout);
        glms_vec3_print(meshes[i].scale, stdout);
    }

    char *directory = realpath("assets/models/main_sponza/", NULL);

    darray(void *) buffers = darray_new(void *);
    for (u32 i = 0; i < darray_length(gltf.buffers); i++) {

        char *file_location = alloca(strlen(directory) + strlen(gltf.buffers[i].uri) + 1);
        strcpy(file_location, directory);
        strcat(file_location, gltf.buffers[i].uri);

        int fd = open(file_location, O_RDONLY);
        void *buffer = mmap(NULL, gltf.buffers[i].byte_length, PROT_READ, 0, fd, 0);
        darray_push(buffers, buffer);
    }

    free(directory);

    if (gltf.buffer_views) {
        LOG_INFO("Buffer Views:");
        for (u32 i = 0; i < darray_length(gltf.buffer_views); i++) {
            LOG_INFO("|-'%s' (%llu, %llu, %d, %u)",
                     gltf.buffer_views[i].name,
                     gltf.buffer_views[i].byte_offset,
                     gltf.buffer_views[i].byte_length,
                     gltf.buffer_views[i].byte_stride,
                     gltf.buffer_views[i].target);
        }
    }

    darray(Model) models = darray_new(Model);
    darray(Texture) textures = darray_new(Texture);

    darray_push(models,
                create_sphere((vec3s){{0, -10000, 0}}, 10000, material_lambertian((vec3s){{0.5f, 0.5f, 0.5f}})));

    for (i32 i = -11; i < 11; i++) {
        for (i32 j = -11; j < 11; j++) {
            f32 mat = randf();
            f32 center_y = (f32)j + 0.9f * randf();
            f32 center_x = (f32)i + 0.9f * randf();
            vec3s center = {{center_x, 0.2f, center_y}};

            if (vec3_distance(center, (vec3s){{4, 0.2f, 0}}) > 0.9f) {
                if (mat < 0.8f) {
                    f32 r = randf() * randf();
                    f32 g = randf() * randf();
                    f32 b = randf() * randf();

                    darray_push(models, create_sphere(center, 0.2f, material_lambertian((vec3s){{r, g, b}})));
                } else if (mat < 0.95) {
                    f32 fuzziness = 0.5f * randf();
                    f32 r = 0.5f * (1 + randf());
                    f32 g = 0.5f * (1 + randf());
                    f32 b = 0.5f * (1 + randf());

                    darray_push(models, create_sphere(center, 0.2f, material_metallic((vec3s){{r, g, b}}, fuzziness)));
                } else {
                    f32 r = randf() * randf();
                    f32 g = randf() * randf();
                    f32 b = randf() * randf();

                    darray_push(models, create_sphere(center, 0.2f, material_diffuse_light((vec3s){{r, g, b}})));
                }
            }
        }
    }

    darray_push(models, create_sphere((vec3s){{0, 1, 0}}, 1.0f, material_dielectric(1.5f)));
    darray_push(models, create_sphere((vec3s){{-4, 1, 0}}, 1.0f, material_lambertian((vec3s){{0.4f, 0.2f, 0.1f}})));
    darray_push(models, create_sphere((vec3s){{4, 1, 0}}, 1.0f, material_metallic((vec3s){{0.7f, 0.6f, 0.5f}}, 0.0f)));

    darray_push(textures, texture_new("assets/textures/white.png"));

    Scene scene = scene_new(models, textures);

    Application application = application_new(window_config, &scene, VK_PRESENT_MODE_IMMEDIATE_KHR, true);

    application_run(&application);

    application_destroy(&application);
}
