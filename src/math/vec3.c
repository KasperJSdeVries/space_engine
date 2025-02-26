#include "vec3.h"

inline vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    }};
}
