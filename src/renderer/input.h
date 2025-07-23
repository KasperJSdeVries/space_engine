#ifndef INPUT_H
#define INPUT_H

#include "core/defines.h"

typedef enum {
    MOVING_FORWARDS = 1 << 0,
    MOVING_BACKWARDS = 1 << 1,
    MOVING_LEFT = 1 << 2,
    MOVING_RIGHT = 1 << 3,
    MOVING_UP = 1 << 4,
    MOVING_DOWN = 1 << 5,
} MovingFlags;

typedef struct {
    u8 moving_flags;
    b8 first_mouse;
    vec2s mouse_pos;
} Input;

void input_set_moving(Input *self, MovingFlags flag, b8 is_moving);

#endif // INPUT_H
