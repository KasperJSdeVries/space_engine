#include "swapchain.h"

#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/device.h"
#include "renderer/surface.h"
#include "renderer/window.h"
#include "vulkan/vulkan_core.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    darray(VkSurfaceFormatKHR) formats;
    darray(VkPresentModeKHR) present_modes;
} SupportDetails;

static SupportDetails query_swapchain_support(VkPhysicalDevice physical_device,
                                              VkSurfaceKHR surface);
static void support_details_destroy(SupportDetails *support_details);
static VkSurfaceFormatKHR choose_swap_surface_format(darray(VkSurfaceFormatKHR)
                                                         formats);
static VkPresentModeKHR choose_swap_present_mode(darray(VkPresentModeKHR)
                                                     present_modes,
                                                 VkPresentModeKHR present_mode);
static VkExtent2D choose_swap_extent(const Window *window,
                                     VkSurfaceCapabilitiesKHR capabilities);
static u32 choose_image_count(VkSurfaceCapabilitiesKHR capabilities);

Swapchain swapchain_new(const Device *device, VkPresentModeKHR present_mode) {
    Swapchain self = {
        .device = device,
    };

    SupportDetails details = query_swapchain_support(device->physical_device,
                                                     device->surface->handle);
    if (darray_length(details.formats) == 0 ||
        darray_length(details.present_modes) == 0) {
        LOG_FATAL("empty swapchain support");
        exit(EXIT_FAILURE);
    }

    const Surface *surface = device->surface;
    const Window *window = surface->instance->window;

    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(details.formats);
    VkPresentModeKHR actual_present_mode =
        choose_swap_present_mode(details.present_modes, present_mode);
    VkExtent2D extent = choose_swap_extent(window, details.capabilities);
    u32 image_count = choose_image_count(details.capabilities);

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface->handle,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = actual_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = NULL,
    };

    u32 queue_family_indices[] = {device->graphics_family_index,
                                  device->present_family_index};
    if (device->graphics_family_index != device->present_family_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    vulkan_check(
        vkCreateSwapchainKHR(device->handle, &create_info, NULL, &self.handle),
        "create swapchain");

    support_details_destroy(&details);

    self.min_image_count = image_count;
    self.present_mode = actual_present_mode;
    self.format = surface_format.format;
    self.extent = extent;
    self.images = darray_new(VkImage);
    self.image_views = darray_new(ImageView);

    u32 swap_image_count;
    vkGetSwapchainImagesKHR(device->handle,
                            self.handle,
                            &swap_image_count,
                            NULL);
    VkImage swap_images[swap_image_count];
    vkGetSwapchainImagesKHR(device->handle,
                            self.handle,
                            &swap_image_count,
                            swap_images);

    for (u32 i = 0; i < swap_image_count; i++) {
        darray_push(self.images, swap_images[i]);
        darray_push(self.image_views,
                    image_view_new(device,
                                   swap_images[i],
                                   self.format,
                                   VK_IMAGE_ASPECT_COLOR_BIT));
    }

    return self;
}

void swapchain_destroy(Swapchain *this) {
    for (u32 i = 0; i < darray_length(this->image_views); i++) {
        image_view_destroy(&this->image_views[i]);
    }

    darray_destroy(this->image_views);
    darray_destroy(this->images);

    if (this->handle != NULL) {
        vkDestroySwapchainKHR(this->device->handle, this->handle, NULL);
        this->handle = NULL;
    }
}

static SupportDetails query_swapchain_support(VkPhysicalDevice physical_device,
                                              VkSurfaceKHR surface) {
    SupportDetails details = {0};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                              surface,
                                              &details.capabilities);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                         surface,
                                         &format_count,
                                         NULL);
    VkSurfaceFormatKHR formats[format_count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                         surface,
                                         &format_count,
                                         formats);

    details.formats = darray_new(VkSurfaceFormatKHR);
    for (u32 i = 0; i < format_count; i++) {
        darray_push(details.formats, formats[i]);
    }

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                              surface,
                                              &present_mode_count,
                                              NULL);
    VkPresentModeKHR present_modes[present_mode_count];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                              surface,
                                              &present_mode_count,
                                              present_modes);

    details.present_modes = darray_new(VkPresentModeKHR);
    for (u32 i = 0; i < present_mode_count; i++) {
        darray_push(details.present_modes, present_modes[i]);
    }

    return details;
}

static void support_details_destroy(SupportDetails *support_details) {
    darray_destroy(support_details->formats);
    darray_destroy(support_details->present_modes);
}

static VkSurfaceFormatKHR choose_swap_surface_format(darray(VkSurfaceFormatKHR)
                                                         formats) {
    if (darray_length(formats) == 1 &&
        formats[0].format == VK_FORMAT_UNDEFINED) {
        return (VkSurfaceFormatKHR){
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };
    }

    for (u32 i = 0; i < darray_length(formats); i++) {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    LOG_FATAL("found no suitable surface format");
    exit(EXIT_FAILURE);
}

static VkPresentModeKHR choose_swap_present_mode(
    darray(VkPresentModeKHR) present_modes,
    VkPresentModeKHR present_mode) {
    switch (present_mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
    case VK_PRESENT_MODE_MAILBOX_KHR:
    case VK_PRESENT_MODE_FIFO_KHR:
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:

        for (u32 i = 0; i < darray_length(present_modes); i++) {
            if (present_modes[i] == present_mode) {
                return present_modes[i];
            }
        }

        break;
    default:
        LOG_FATAL("unknown present mode");
        exit(EXIT_FAILURE);
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(const Window *window,
                                     VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = window_framebuffer_size(window);

    actual_extent.width = CLAMP(actual_extent.width,
                                capabilities.minImageExtent.width,
                                capabilities.maxImageExtent.width);
    actual_extent.height = CLAMP(actual_extent.height,
                                 capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height);

    return actual_extent;
}

static u32 choose_image_count(VkSurfaceCapabilitiesKHR capabilities) {
    u32 image_count =
        CLAMP(2, capabilities.minImageCount, capabilities.maxImageCount);

    return image_count;
}
