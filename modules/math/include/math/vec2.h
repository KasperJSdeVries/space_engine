#ifndef SE_VEC2_H
#define SE_VEC2_H

#include "core/assert.h"
#include "core/defines.h"

typedef union {
    struct {
        f32 x, y;
    };
    struct {
        f32 u, v;
    };
    f32 raw[2];
} vec2;

#define VEC2_FORMAT "[%f %f]"
#define VEC2_PRINT(vec) (vec).x, (vec).y

static inline vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){{
        a.x + b.x,
        a.y + b.y,
    }};
}

static inline vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2){{
        a.x - b.x,
        a.y - b.y,
    }};
}

static inline vec2 vec2_mul(vec2 a, vec2 b) {
    return (vec2){{
        a.x * b.x,
        a.y * b.y,
    }};
}

static inline vec2 vec2_scale(vec2 vec, f32 s) {
    return (vec2){{
        vec.x * s,
        vec.y * s,
    }};
}

static inline vec2 vec2_div(vec2 a, vec2 b) {
    ASSERT(b.x != 0.0f && b.y != 0.0f);
    return (vec2){{
        a.x / b.x,
        a.y / b.y,
    }};
}

static inline f32 vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }

#endif // SE_VEC2_H
