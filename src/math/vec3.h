#ifndef SE_VEC3_H
#define SE_VEC3_H

#include "core/assert.h"
#include "core/defines.h"


typedef union {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    struct {
        f32 u, v, w;
    };
    f32 raw[3];
} vec3;

#define vec3_format "[%f %f %f]"
#define vec3_print(vec) (vec).x, (vec).y, (vec).z

static inline vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    }};
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3){{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    }};
}

static inline vec3 vec3_mul(vec3 a, vec3 b) {
    return (vec3){{
        a.x * b.x,
        a.y * b.y,
        a.z * b.z,
    }};
}

static inline vec3 vec3_scale(vec3 vec, f32 s) {
    return (vec3){{
        vec.x * s,
        vec.y * s,
        vec.z * s,
    }};
}

static inline vec3 vec3_div(vec3 a, vec3 b) {
    ASSERT(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f);
    return (vec3){{
        a.x / b.x,
        a.y / b.y,
        a.z / b.z,
    }};
}

static inline f32 vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static inline vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x / b.y - a.y * b.x,
    };
}

#endif // SE_VEC3_H
