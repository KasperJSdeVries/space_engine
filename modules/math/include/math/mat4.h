#ifndef SE_MAT4_H
#define SE_MAT4_H

#include "math/vec3.h"
#include "math/vec4.h"

#include <math.h>

typedef union {
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };
    vec4 col[4];
    f32 raw[16];
} mat4;

#define MAT4_FORMAT "\n" VEC4_FORMAT "\n" VEC4_FORMAT "\n" VEC4_FORMAT "\n" VEC4_FORMAT "\n"
#define MAT4_PRINT(m)                                                                              \
    VEC4_PRINT((m).col[0]), VEC4_PRINT((m).col[1]), VEC4_PRINT((m).col[2]), VEC4_PRINT((m).col[3])

#define MAT4_ZERO (mat4){0}
#define MAT4_IDENTITY (mat4){.m00 = 1.0f, .m11 = 1.0f, .m22 = 1.0f, .m33 = 1.0f}

static inline mat4 rotate_make(float angle, vec3 axis) {
    mat4 m = {0};

    f32 c = cosf(angle);

    vec3 axis_normalized = vec3_normalize(axis);
    vec3 v = vec3_scale(axis_normalized, 1.0f - c);
    vec3 vs = vec3_scale(axis_normalized, sinf(angle));

    m.col[0] = vec4_from_vec3(vec3_scale(axis_normalized, v.x), 0.0f);
    m.col[1] = vec4_from_vec3(vec3_scale(axis_normalized, v.y), 0.0f);
    m.col[2] = vec4_from_vec3(vec3_scale(axis_normalized, v.z), 0.0f);

    m.m00 += c;
    m.m10 -= vs.z;
    m.m20 += vs.y;
    m.m01 += vs.z;
    m.m11 += c;
    m.m21 -= vs.x;
    m.m02 -= vs.y;
    m.m12 += vs.x;
    m.m22 += c;

    m.m33 = 1.0f;

    return m;
}

static inline mat4 mul_rot(mat4 m1, mat4 m2) {
    return (mat4){
        .m00 = m1.m00 * m2.m00 + m1.m10 * m2.m01 + m1.m20 * m2.m02,
        .m01 = m1.m01 * m2.m00 + m1.m11 * m2.m01 + m1.m21 * m2.m02,
        .m02 = m1.m02 * m2.m00 + m1.m12 * m2.m01 + m1.m22 * m2.m02,
        .m03 = m1.m03 * m2.m00 + m1.m13 * m2.m01 + m1.m23 * m2.m02,

        .m10 = m1.m00 * m2.m10 + m1.m10 * m2.m11 + m1.m20 * m2.m12,
        .m11 = m1.m01 * m2.m10 + m1.m11 * m2.m11 + m1.m21 * m2.m12,
        .m12 = m1.m02 * m2.m10 + m1.m12 * m2.m11 + m1.m22 * m2.m12,
        .m13 = m1.m03 * m2.m10 + m1.m13 * m2.m11 + m1.m23 * m2.m12,

        .m20 = m1.m00 * m2.m20 + m1.m10 * m2.m21 + m1.m20 * m2.m22,
        .m21 = m1.m01 * m2.m20 + m1.m11 * m2.m21 + m1.m21 * m2.m22,
        .m22 = m1.m02 * m2.m20 + m1.m12 * m2.m21 + m1.m22 * m2.m22,
        .m23 = m1.m03 * m2.m20 + m1.m13 * m2.m21 + m1.m23 * m2.m22,

        .m30 = m1.m30,
        .m31 = m1.m31,
        .m32 = m1.m32,
        .m33 = m1.m33,
    };
}

static inline mat4 rotate(mat4 m, f32 angle, vec3 axis) {
    return mul_rot(m, rotate_make(angle, axis));
}

static inline mat4 lookat(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_sub(center, eye);
    f = vec3_normalize(f);

    vec3 s = vec3_normalize(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    return (mat4){
        .m00 = s.x,
        .m01 = u.x,
        .m02 = -f.x,

        .m10 = s.y,
        .m11 = u.y,
        .m12 = -f.y,

        .m20 = s.z,
        .m21 = u.z,
        .m22 = -f.z,

        .m30 = -vec3_dot(s, eye),
        .m31 = -vec3_dot(u, eye),
        .m32 = vec3_dot(f, eye),

        .m33 = 1.0f,
    };
}

static inline mat4 perspective(f32 fovy, f32 aspect, f32 near_z, f32 far_z) {
    f32 f = 1.0f / tanf(fovy * 0.5f);
    f32 fn = 1.0f / (near_z - far_z);

    return (mat4){
        .m00 = f / aspect,
        .m11 = f,
        .m22 = (near_z + far_z) * fn,
        .m23 = -1.0f,
        .m32 = 2.0f * near_z * far_z * fn,
    };
}

#endif // SE_MAT4_H
