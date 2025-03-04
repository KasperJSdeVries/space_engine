#include "render.h"
#include "render_system/device.h"
#include "render_system/instance.h"

static struct render_system_state state = {0};

b8 create_instance(void);
void destroy_instance(void);

b8 render_system_startup(void) {
    if (!instance_create(&state.instance)) {
        return false;
    }

    if (!device_create(&state.instance, &state.device)) {
        return false;
    }

    return true;
}

void render_system_shutdown(void) {
    device_destroy(&state.instance, &state.device);
    instance_destroy(&state.instance);
}
