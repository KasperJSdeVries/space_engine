#include "input.h"

void input_set_moving(Input *self, MovingFlags flag, b8 is_moving) {
    if (is_moving) {
        self->moving_flags |= flag;
    } else {
        self->moving_flags &= ~flag;
    }
}
