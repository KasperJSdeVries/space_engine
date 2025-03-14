#ifndef SE_MAT4_H
#define SE_MAT4_H

#include "math/vec4.h"

typedef union {
	struct {
		f32 m00,m01,m02,m03;
		f32 m10,m11,m12,m13;
		f32 m20,m21,m22,m23;
		f32 m30,m31,m32,m33;
	};
    vec4 col[4];
    f32 raw[16];
} mat4;

#endif // SE_MAT4_H
