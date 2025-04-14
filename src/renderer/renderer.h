#ifndef RENDER_H
#define RENDER_H

#include "core/result.h"
#include "types.h"

se_result renderer_startup(Display *display, struct renderer *renderer);
void renderer_shutdown(struct renderer *renderer);
se_result renderer_start_frame(struct renderer *renderer);
se_result renderer_end_frame(struct renderer *renderer);
void renderer_end_main_loop(const struct renderer *renderer);

#endif // RENDER_H
