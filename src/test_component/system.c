#include "system.h"
#include "component.h"

#include <stdio.h>

void declare_system(void) {
    test_component component = {
        .size = 10,
        .angles = 3,
    };
    printf("size: %f, angles: %d\n", component.size, component.angles);
    return;
}
