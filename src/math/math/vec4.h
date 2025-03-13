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

inline vec4 vec4_add(vec4 a, vec4 b);

#endif // SE_VEC4_H
