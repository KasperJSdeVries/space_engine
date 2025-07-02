#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "renderer/render_pass.h"

typedef struct {
    VkFramebuffer handle;
    const ImageView *image_view;
    const RenderPass *render_pass;
} Framebuffer;

Framebuffer framebuffer_new(const ImageView *image_view,
                            const RenderPass *render_pass);
Framebuffer framebuffer_move(Framebuffer *other);
void framebuffer_destroy(Framebuffer *self);

#endif // FRAMEBUFFER_H
