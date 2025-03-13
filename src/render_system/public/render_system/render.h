#ifndef RENDER_H
#define RENDER_H

#include "platform/platform.h"

b8 render_system_startup(struct se_window *window);
void render_system_shutdown(void);
b8 render_system_render_frame(const struct se_window *window);

#endif // RENDER_H
