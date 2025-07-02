#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "depth_buffer.h"
#include "swapchain.h"

typedef struct {
    VkRenderPass handle;
    const Swapchain *swapchain;
    const DepthBuffer *depth_buffer;
} RenderPass;

RenderPass render_pass_new(const Swapchain *swapchain,
                           const DepthBuffer *depth_buffer,
                           VkAttachmentLoadOp color_buffer_load_op,
                           VkAttachmentLoadOp depth_buffer_load_op);
void render_pass_destroy(RenderPass *self);

#endif // RENDER_PASS_H
