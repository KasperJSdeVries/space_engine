#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "platform/platform.h"

b8 render_system_startup(const struct se_window *window);
void render_system_shutdown(void);

#endif // RENDER_H
