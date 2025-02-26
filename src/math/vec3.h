#ifndef SE_VEC3_H
#define SE_VEC3_H

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

inline vec3 vec3_add(vec3 a, vec3 b);

#endif // SE_VEC3_H
