#ifndef RENDER_H
#define RENDER_H

#include "platform/platform.h"

b8 render_system_startup(const struct se_window *window);
void render_system_shutdown(void);
b8 render_system_render_frame(void) ;

#endif // RENDER_H
