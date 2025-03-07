#include "swapchain.h"

#include "core/assert.h"
#include "core/defines.h"
#include "platform/platform.h"
#include "render_system/device.h"
#include "render_system/image.h"
#include "render_system/types.h"
#include "vulkan/vulkan_core.h"

#include <stdlib.h>

struct swapchain_support_details query_swapchain_support(VkPhysicalDevice device,
                                                         VkSurfaceKHR surface) {
    struct swapchain_support_details details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.format_count, NULL);
    if (details.format_count != 0) {
        details.formats = calloc(details.format_count, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             surface,
                                             &details.format_count,
                                             details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.present_mode_count, NULL);
    if (details.present_mode_count != 0) {
        details.present_modes = calloc(details.present_mode_count, sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                  surface,
                                                  &details.present_mode_count,
                                                  details.present_modes);
    }

    return details;
}

void swapchain_support_details_destroy(struct swapchain_support_details *details) {
    free(details->formats);
    details->formats = NULL;
    free(details->present_modes);
    details->present_modes = NULL;
}

VkSurfaceFormatKHR choose_swap_surface_format(u32 available_format_count,
                                              const VkSurfaceFormatKHR *available_formats) {
    for (u32 i = 0; i < available_format_count; i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(u32 available_present_mode_count,
                                          const VkPresentModeKHR *available_present_modes) {
    for (u32 i = 0; i < available_present_mode_count; i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return available_present_modes[i];
        }
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities,
                              const struct se_window *window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D extent;
        platform_get_framebuffer_size(window, &extent);

        extent.width = CLAMP(extent.width,
                             capabilities->minImageExtent.width,
                             capabilities->maxImageExtent.width);
        extent.height = CLAMP(extent.height,
                              capabilities->minImageExtent.height,
                              capabilities->maxImageExtent.height);

        return extent;
    }
}

b8 swapchain_create(const struct device *device,
                    VkSurfaceKHR surface,
                    const struct se_window *window,
                    struct swapchain *swapchain) {
    struct swapchain_support_details swapchain_support =
        query_swapchain_support(device->physical_device, surface);

    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swapchain_support.format_count, swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_mode_count,
                                                             swapchain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(&swapchain_support.capabilities, window);

    u32 image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchain_support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
    };

    struct queue_family_indices indices = find_queue_families(device->physical_device, surface);
    u32 queue_family_indices[] = {indices.graphics_family, indices.present_family};
    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchain_support_details_destroy(&swapchain_support);

    if (!ASSERT(vkCreateSwapchainKHR(device->handle, &create_info, NULL, &swapchain->handle) ==
                VK_SUCCESS)) {
        return false;
    }

    swapchain->format = surface_format.format;
    swapchain->extent = extent;

    vkGetSwapchainImagesKHR(device->handle, swapchain->handle, &swapchain->image_count, NULL);
    swapchain->images = calloc(swapchain->image_count, sizeof(VkImage));
    vkGetSwapchainImagesKHR(device->handle,
                            swapchain->handle,
                            &swapchain->image_count,
                            swapchain->images);

    swapchain->image_views = calloc(swapchain->image_count, sizeof(VkImageView));
    for (u32 i = 0; i < swapchain->image_count; i++) {
        image_view_create(device,
                          swapchain->images[i],
                          surface_format.format,
                          &swapchain->image_views[i]);
    }

    return true;
}

void swapchain_destroy(const struct device *device, struct swapchain *swapchain) {
    for (u32 i = 0; i < swapchain->image_count; i++) {
        vkDestroyImageView(device->handle, swapchain->image_views[i], NULL);
    }

    vkDestroySwapchainKHR(device->handle, swapchain->handle, NULL);
    swapchain->handle = VK_NULL_HANDLE;

    free(swapchain->images);
    swapchain->images = NULL;
    swapchain->image_count = 0;
}

b8 swapchain_framebuffers_create(const struct device *device,
                                 const struct renderpass *renderpass,
                                 struct swapchain *swapchain) {
    swapchain->framebuffers = calloc(swapchain->image_count, sizeof(VkFramebuffer));

    for (u32 i = 0; i < swapchain->image_count; i++) {
        VkImageView attachments[] = {
            swapchain->image_views[i],
        };

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderpass->handle,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain->extent.width,
            .height = swapchain->extent.height,
            .layers = 1,
        };

        if (!ASSERT(vkCreateFramebuffer(device->handle,
                                        &framebuffer_info,
                                        NULL,
                                        &swapchain->framebuffers[i]) == VK_SUCCESS)) {
            return false;
        }
    }

    return true;
}

void swapchain_framebuffers_destroy(const struct device *device, struct swapchain *swapchain) {
    for (u32 i = 0; i < swapchain->image_count; i++) {
        vkDestroyFramebuffer(device->handle, swapchain->framebuffers[i], NULL);
    }

    free(swapchain->framebuffers);
    swapchain->framebuffers = NULL;
}
