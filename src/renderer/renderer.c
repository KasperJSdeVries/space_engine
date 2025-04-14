#include "renderer.h"

#include "commandbuffer.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/result.h"
#include "device.h"
#include "instance.h"
#include "renderpass.h"
#include "swapchain.h"
#include "types.h"
#include "window.h"

#include "X11/Xlib.h"
#include "vulkan/vulkan_core.h"

#include <string.h>

void on_window_resize(void);

se_result renderer_startup(Display *display, struct renderer *renderer) {
    se_result result;

    if (!ASSERT(instance_create(&renderer->instance))) {
        return false;
    }

    if (!ASSERT(result = window_create(display, &renderer->window) ==
                         SE_RESULT_OK)) {
        return result;
    }

    if (!ASSERT(result = window_surface_create(&renderer->window,
                                               &renderer->instance,
                                               &renderer->surface) ==
                         SE_RESULT_OK)) {
        return result;
    }

    if (!ASSERT(device_create(&renderer->instance,
                              &renderer->surface,
                              &renderer->device))) {
        return SE_RESULT_VULKAN_ERROR;
    }

    if (!swapchain_create(&renderer->device,
                          &renderer->surface,
                          &renderer->window,
                          &renderer->swapchain)) {
        return SE_RESULT_VULKAN_ERROR;
    }

    if (!renderpass_create(&renderer->device,
                           &renderer->swapchain,
                           &renderer->renderpass)) {
        return SE_RESULT_VULKAN_ERROR;
    }

    if (!swapchain_framebuffers_create(&renderer->device,
                                       &renderer->renderpass,
                                       &renderer->swapchain)) {
        return SE_RESULT_VULKAN_ERROR;
    }

    if (!commandpool_create(&renderer->device,
                            &renderer->surface,
                            &renderer->commandpool)) {
        return SE_RESULT_VULKAN_ERROR;
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!commandbuffer_create(&renderer->device,
                                  &renderer->commandpool,
                                  &renderer->commandbuffers[i])) {
            return SE_RESULT_VULKAN_ERROR;
        }
    }

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!ASSERT(
                vkCreateSemaphore(renderer->device.handle,
                                  &semaphore_info,
                                  NULL,
                                  &renderer->image_available_semaphores[i]) ==
                VK_SUCCESS) ||
            !ASSERT(
                vkCreateSemaphore(renderer->device.handle,
                                  &semaphore_info,
                                  NULL,
                                  &renderer->render_finished_semaphores[i]) ==
                VK_SUCCESS) ||
            !ASSERT(vkCreateFence(renderer->device.handle,
                                  &fence_info,
                                  NULL,
                                  &renderer->in_flight_fences[i]) ==
                    VK_SUCCESS)) {
            return false;
        }
    }

    return SE_RESULT_OK;
}

void renderer_shutdown(struct renderer *renderer) {
    vkDeviceWaitIdle(renderer->device.handle);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(renderer->device.handle,
                       renderer->in_flight_fences[i],
                       NULL);
        vkDestroySemaphore(renderer->device.handle,
                           renderer->render_finished_semaphores[i],
                           NULL);
        vkDestroySemaphore(renderer->device.handle,
                           renderer->image_available_semaphores[i],
                           NULL);
    }
    commandpool_destroy(&renderer->device, &renderer->commandpool);
    swapchain_framebuffers_destroy(&renderer->device, &renderer->swapchain);
    renderpass_destroy(&renderer->device, &renderer->renderpass);
    swapchain_destroy(&renderer->device, &renderer->swapchain);
    device_destroy(&renderer->device);
    window_destroy(&renderer->window);
    vkDestroySurfaceKHR(renderer->instance.handle,
                        renderer->surface.handle,
                        NULL);
}

se_result renderer_start_frame(struct renderer *renderer) {
    se_result result;
    if ((result = window_update(&renderer->window)) ==
        SE_RESULT_WINDOW_RESIZED) {
        renderer->framebuffer_resized = true;
    } else if (result != SE_RESULT_OK) {
        return result;
    }

    vkWaitForFences(renderer->device.handle,
                    1,
                    &renderer->in_flight_fences[renderer->current_frame],
                    VK_TRUE,
                    UINT64_MAX);

    VkResult acquire_next_image_result = vkAcquireNextImageKHR(
        renderer->device.handle,
        renderer->swapchain.handle,
        UINT64_MAX,
        renderer->image_available_semaphores[renderer->current_frame],
        VK_NULL_HANDLE,
        &renderer->image_index);

    if (acquire_next_image_result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain_recreate(&renderer->device,
                           &renderer->surface,
                           &renderer->window,
                           &renderer->renderpass,
                           &renderer->swapchain);
        return true;
    } else if (!ASSERT(acquire_next_image_result == VK_SUCCESS ||
                       acquire_next_image_result == VK_SUBOPTIMAL_KHR)) {
        return false;
    }

    vkResetFences(renderer->device.handle,
                  1,
                  &renderer->in_flight_fences[renderer->current_frame]);

    vkResetCommandBuffer(
        renderer->commandbuffers[renderer->current_frame].handle,
        0);
    if (!commandbuffer_recording_start(
            &renderer->commandbuffers[renderer->current_frame])) {
        return false;
    }

    renderpass_begin(&renderer->renderpass,
                     &renderer->commandbuffers[renderer->current_frame],
                     &renderer->swapchain,
                     renderer->image_index);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = renderer->swapchain.extent.width,
        .height = renderer->swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(renderer->commandbuffers[renderer->current_frame].handle,
                     0,
                     1,
                     &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = renderer->swapchain.extent,
    };
    vkCmdSetScissor(renderer->commandbuffers[renderer->current_frame].handle,
                    0,
                    1,
                    &scissor);

    return SE_RESULT_OK;
}

se_result renderer_end_frame(struct renderer *renderer) {
    renderpass_end(&renderer->commandbuffers[renderer->current_frame]);

    if (!commandbuffer_recording_end(
            &renderer->commandbuffers[renderer->current_frame])) {
        return false;
    }

    VkSemaphore wait_semaphores[] = {
        renderer->image_available_semaphores[renderer->current_frame]};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {
        renderer->render_finished_semaphores[renderer->current_frame]};

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = ARRAY_SIZE(wait_semaphores),
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers =
            &renderer->commandbuffers[renderer->current_frame].handle,
        .signalSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pSignalSemaphores = signal_semaphores,
    };

    if (!ASSERT(vkQueueSubmit(
                    renderer->device.graphics_queue,
                    1,
                    &submit_info,
                    renderer->in_flight_fences[renderer->current_frame]) ==
                VK_SUCCESS)) {
        return false;
    }

    VkSwapchainKHR swapchains[] = {renderer->swapchain.handle};

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = ARRAY_SIZE(swapchains),
        .pSwapchains = swapchains,
        .pImageIndices = &renderer->image_index,
        .pResults = NULL,
    };

    VkResult result =
        vkQueuePresentKHR(renderer->device.present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        renderer->framebuffer_resized) {
        renderer->framebuffer_resized = false;
        swapchain_recreate(&renderer->device,
                           &renderer->surface,
                           &renderer->window,
                           &renderer->renderpass,
                           &renderer->swapchain);
    } else if (!ASSERT(result == VK_SUCCESS)) {
        return false;
    }

    renderer->current_frame =
        (renderer->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return SE_RESULT_OK;
}

void renderer_end_main_loop(const struct renderer *renderer) {
    vkDeviceWaitIdle(renderer->device.handle);
}
