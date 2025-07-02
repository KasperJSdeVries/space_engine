#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "device.h"
#include "image_view.h"

typedef struct {
    VkSwapchainKHR handle;
    const Device *device;
    darray(VkImage) images;
    darray(ImageView) image_views;
    VkExtent2D extent;
    u32 min_image_count;
    VkPresentModeKHR present_mode;
    VkFormat format;
} Swapchain;

Swapchain swapchain_new(const Device *device, VkPresentModeKHR present_mode);
void swapchain_destroy(Swapchain *swapchain);

#endif // SWAPCHAIN_H
