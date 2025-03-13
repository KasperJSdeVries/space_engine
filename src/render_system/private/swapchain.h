#ifndef RENDER_SWAPCHAIN_H
#define RENDER_SWAPCHAIN_H

#include "types.h"

#include "platform/platform.h"

struct swapchain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *present_modes;
    u32 format_count;
    u32 present_mode_count;
};

struct swapchain_support_details query_swapchain_support(VkPhysicalDevice device,
                                                         VkSurfaceKHR surface);
void swapchain_support_details_destroy(struct swapchain_support_details *details);

VkSurfaceFormatKHR choose_swap_surface_format(u32 available_format_count,
                                              const VkSurfaceFormatKHR *available_formats);
VkPresentModeKHR choose_swap_present_mode(u32 available_present_mode_count,
                                          const VkPresentModeKHR *available_present_modes);
VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities,
                              const struct se_window *window);

b8 swapchain_create(const struct device *device,
                    VkSurfaceKHR surface,
                    const struct se_window *window,
                    struct swapchain *swapchain);
void swapchain_destroy(const struct device *device, struct swapchain *swapchain);
b8 swapchain_recreate(const struct device *device,
                      VkSurfaceKHR surface,
                      const struct se_window *window,
                      const struct renderpass *renderpass,
                      struct swapchain *swapchain);

b8 swapchain_framebuffers_create(const struct device *device,
                                 const struct renderpass *renderpass,
                                 struct swapchain *swapchain);
void swapchain_framebuffers_destroy(const struct device *device, struct swapchain *swapchain);

#endif // RENDER_SWAPCHAIN_H
