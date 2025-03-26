#ifndef SE_VEC3_H
#define SE_VEC3_H

#include "core/assert.h"
#include "core/defines.h"

#include <math.h>

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

#define VEC3_FORMAT "[%f %f %f]"
#define VEC3_PRINT(vec) (vec).x, (vec).y, (vec).z

#define VEC3_ZERO                                                                                  \
    (vec3) {{0.0f,0.0f,0.0f}}
#define VEC3_ONE                                                                                   \
    (vec3) {{1.0f,1.0f,1.0f}}

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

static inline f32 vec3_norm2(vec3 v) { return vec3_dot(v, v); }

static inline f32 vec3_norm(vec3 v) { return sqrtf(vec3_norm2(v)); }

static inline vec3 vec3_normalize(vec3 v) {
    f32 norm = vec3_norm(v);

    if (UNLIKELY(norm < F32_EPSILON)) {
        return VEC3_ZERO;
    }

    return vec3_scale(v, norm);
}

#endif // SE_VEC3_H
