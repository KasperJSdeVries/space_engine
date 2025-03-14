#ifndef RENDER_RENDERPASS_H
#define RENDER_RENDERPASS_H

#include "types.h"

b8 renderpass_create(const struct device *device,
                     const struct swapchain *swapchain,
                     struct renderpass *renderpass);
void renderpass_destroy(const struct device *device, struct renderpass *renderpass);
void renderpass_begin(const struct renderpass *renderpass,
                      const struct commandbuffer *commandbuffer,
                      const struct swapchain *swapchain,
                      u32 image_index);
void renderpass_end(const struct commandbuffer *commandbuffer);

#endif // RENDER_RENDERPASS_H
