#ifndef SE_VEC4_H
#define SE_VEC4_H

#include "core/defines.h"
#include "math/vec3.h"

typedef union {
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
    f32 raw[4];
} vec4;

typedef union {
    struct {
        f32 x, y, z, w;
    };
    struct {
        vec3 imaginary;
        f32 real;
    };
    f32 raw[4];
} versors;

#define VEC4_FORMAT "[%f %f %f %f]"
#define VEC4_PRINT(vec) (vec).x, (vec).y, (vec).z, (vec).w

static inline vec4 vec4_from_vec3(vec3 v, f32 f) { return (vec4){{v.x, v.y, v.z, f}}; }

static inline vec4 vec4_add(vec4 a, vec4 b) {
    return (vec4){{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w,
    }};
}

static inline vec4 vec4_sub(vec4 a, vec4 b) {
    return (vec4){{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
        a.w - b.w,
    }};
}

static inline vec4 vec4_mul(vec4 a, vec4 b) {
    return (vec4){{
        a.x * b.x,
        a.y * b.y,
        a.z * b.z,
        a.w * b.w,
    }};
}

static inline vec4 vec4_scale(vec4 vec, f32 s) {
    return (vec4){{
        vec.x * s,
        vec.y * s,
        vec.z * s,
        vec.w * s,
    }};
}

static inline vec4 vec4_div(vec4 a, vec4 b) {
    ASSERT(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f && b.w != 0.0f);
    return (vec4){{
        a.x / b.x,
        a.y / b.y,
        a.z / b.z,
        a.w / b.w,
    }};
}

static inline f32 vec4_dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

#endif // SE_VEC4_H
