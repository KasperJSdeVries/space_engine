#ifndef MATERIAL_H
#define MATERIAL_H

#include "cglm/types-struct.h"
#include "core/defines.h"

#include <stdalign.h>

typedef enum {
    MATERIAL_MODEL_LAMBERTIAN = 0,
    MATERIAL_MODEL_METALLIC = 1,
    MATERIAL_MODEL_DIELECTRIC = 2,
    MATERIAL_MODEL_ISOTROPIC = 3,
    MATERIAL_MODEL_DIFFUSE_LIGHT = 4,
} MaterialModel;

typedef struct {
    vec4s diffuse;
    i32 diffuse_texture_id;
    f32 fuzziness;
    f32 refraction_index;
    u32 material_model;
} Material;

inline static Material material_lambertian(vec3s diffuse) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        -1,
        0.0f,
        0.0f,
        MATERIAL_MODEL_LAMBERTIAN,
    };
}

inline static Material material_lambertian_textured(vec3s diffuse,
                                                    i32 texture_id) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        texture_id,
        0.0f,
        0.0f,
        MATERIAL_MODEL_LAMBERTIAN,
    };
}

inline static Material material_metallic(vec3s diffuse, f32 fuzziness) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        -1,
        fuzziness,
        0.0f,
        MATERIAL_MODEL_METALLIC,
    };
}

inline static Material material_metallic_textured(vec3s diffuse,
                                                  f32 fuzziness,
                                                  i32 texture_id) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        texture_id,
        fuzziness,
        0.0f,
        MATERIAL_MODEL_METALLIC,
    };
}

inline static Material material_dielectric(f32 refraction_index) {
    return (Material){
        (vec4s){{0.7, 0.7, 1.0f, 1.0f}},
        -1,
        0.0f,
        refraction_index,
        MATERIAL_MODEL_DIELECTRIC,
    };
}

inline static Material material_dielectric_textured(f32 refraction_index,
                                                    i32 texture_id) {
    return (Material){
        (vec4s){{0.7, 0.7, 1.0f, 1.0f}},
        texture_id,
        0.0f,
        refraction_index,
        MATERIAL_MODEL_DIELECTRIC,
    };
}

inline static Material material_isotropic(vec3s diffuse) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        -1,
        0.0f,
        0.0f,
        MATERIAL_MODEL_ISOTROPIC,
    };
}

inline static Material material_isotropic_textured(vec3s diffuse,
                                                   i32 texture_id) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        texture_id,
        0.0f,
        0.0f,
        MATERIAL_MODEL_ISOTROPIC,
    };
}
inline static Material material_diffuse_light(vec3s diffuse) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        -1,
        0.0f,
        0.0f,
        MATERIAL_MODEL_DIFFUSE_LIGHT,
    };
}

inline static Material material_diffuse_light_textured(vec3s diffuse,
                                                       i32 texture_id) {
    return (Material){
        glms_vec4(diffuse, 1.0f),
        texture_id,
        0.0f,
        0.0f,
        MATERIAL_MODEL_DIFFUSE_LIGHT,
    };
}
#endif // MATERIAL_H
