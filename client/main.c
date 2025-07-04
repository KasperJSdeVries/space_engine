#include "assets/material.h"
#include "core/defines.h"
#include "renderer/application.h"

static f32 randf(void) { return (f32)random() / (f32)UINT32_MAX; }

int main(void) {
    WindowConfig window_config = {
        .title = "Vulkan Window",
        .width = 1080,
        .height = 720,
        .cursor_disabled = false,
        .fullscreen = false,
        .resizable = true,
    };

    darray(Model) models = darray_new(Model);
    darray(Texture) textures = darray_new(Texture);

    darray_push(
        models,
        create_sphere((vec3s){{0, -1000, 0}},
                      1000,
                      material_lambertian((vec3s){{0.5f, 0.5f, 0.5f}})));

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

                    darray_push(
                        models,
                        create_sphere(center,
                                      0.2f,
                                      material_lambertian((vec3s){{r, g, b}})));
                } else if (mat < 0.95) {
                    f32 fuzziness = 0.5f * randf();
                    f32 r = 0.5f * (1 + randf());
                    f32 g = 0.5f * (1 + randf());
                    f32 b = 0.5f * (1 + randf());

                    darray_push(
                        models,
                        create_sphere(
                            center,
                            0.2f,
                            material_metallic((vec3s){{r, g, b}}, fuzziness)));
                } else {
                    darray_push(
                        models,
                        create_sphere(center, 0.2f, material_dielectric(1.5f)));
                }
            }
        }
    }

    darray_push(
        models,
        create_sphere((vec3s){{0, 1, 0}}, 1.0f, material_dielectric(1.5f)));
    darray_push(
        models,
        create_sphere((vec3s){{-4, 1, 0}},
                      1.0f,
                      material_lambertian((vec3s){{0.4f, 0.2f, 0.1f}})));
    darray_push(
        models,
        create_sphere((vec3s){{4, 1, 0}},
                      1.0f,
                      material_metallic((vec3s){{0.7f, 0.6f, 0.5f}}, 0.0f)));

    darray_push(textures, texture_new("assets/textures/white.png"));

    Scene scene = scene_new(models, textures);

    Application application = application_new(window_config,
                                              &scene,
                                              VK_PRESENT_MODE_IMMEDIATE_KHR,
                                              true);

    application_run(&application);

    application_destroy(&application);
}
