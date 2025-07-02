#include "framebuffer.h"
#include "core/defines.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

Framebuffer framebuffer_new(const ImageView *image_view,
                            const RenderPass *render_pass) {
    Framebuffer self = {
        .image_view = image_view,
        .render_pass = render_pass,
    };

    VkImageView attachments[] = {
        image_view->handle,
        render_pass->depth_buffer->image_view->handle,
    };

    VkFramebufferCreateInfo framebuffer_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass->handle,
        .attachmentCount = ARRAY_SIZE(attachments),
        .pAttachments = attachments,
        .width = render_pass->swapchain->extent.width,
        .height = render_pass->swapchain->extent.height,
        .layers = 1,
    };

    vulkan_check(vkCreateFramebuffer(image_view->device->handle,
                                     &framebuffer_info,
                                     NULL,
                                     &self.handle),
                 "create framebuffer");

    return self;
}

Framebuffer framebuffer_move(Framebuffer *other) {
    Framebuffer self = *other;

    other->handle = NULL;

    return self;
}

void framebuffer_destroy(Framebuffer *self) {
    if (self->handle != NULL) {
        vkDestroyFramebuffer(self->image_view->device->handle,
                             self->handle,
                             NULL);
        self->handle = NULL;
    }
}
