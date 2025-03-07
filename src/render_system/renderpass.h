#ifndef RENDER_RENDERPASS_H
#define RENDER_RENDERPASS_H

#include "types.h"

b8 renderpass_create(const struct device *device,
                     const struct swapchain *swapchain,
                     struct renderpass *renderpass);
void renderpass_destroy(const struct device *device, struct renderpass *renderpass);

#endif // RENDER_RENDERPASS_H
