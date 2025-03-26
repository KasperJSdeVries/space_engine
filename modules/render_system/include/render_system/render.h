#ifndef RENDER_H
#define RENDER_H

#include "platform/platform.h"

struct renderer;

b8 render_system_startup(struct se_window *window, struct renderer *renderer);
void render_system_shutdown(struct renderer *renderer);
b8 render_system_start_frame(const struct se_window *window, struct renderer *renderer);
b8 render_system_end_frame(const struct se_window *window, struct renderer *renderer);

#endif // RENDER_H
