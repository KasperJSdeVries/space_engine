#include "render.h"

#include "commandbuffer.h"
#include "device.h"
#include "instance.h"
#include "renderpass.h"
#include "swapchain.h"
#include "types.h"

#include "core/assert.h"
#include "core/defines.h"

#include "vulkan/vulkan_core.h"
#include <string.h>

static struct renderer *state_ptr = NULL;

void on_window_resize(void);

b8 render_system_startup(struct se_window *window, struct renderer *renderer) {
    state_ptr = renderer;

    if (!instance_create(&renderer->instance)) {
        return false;
    }

    if (!platform_surface_create(window, renderer->instance.handle, &renderer->surface)) {
        return false;
    }

    if (!device_create(&renderer->instance, renderer->surface, &renderer->device)) {
        return false;
    }

    if (!swapchain_create(&renderer->device, renderer->surface, window, &renderer->swapchain)) {
        return false;
    }

    if (!renderpass_create(&renderer->device, &renderer->swapchain, &renderer->renderpass)) {
        return false;
    }

    if (!swapchain_framebuffers_create(&renderer->device,
                                       &renderer->renderpass,
                                       &renderer->swapchain)) {
        return false;
    }

    if (!commandpool_create(&renderer->device, renderer->surface, &renderer->commandpool)) {
        return false;
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!commandbuffer_create(&renderer->device,
                                  &renderer->commandpool,
                                  &renderer->commandbuffers[i])) {
            return false;
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
        if (!ASSERT(vkCreateSemaphore(renderer->device.handle,
                                      &semaphore_info,
                                      NULL,
                                      &renderer->image_available_semaphores[i]) == VK_SUCCESS) ||
            !ASSERT(vkCreateSemaphore(renderer->device.handle,
                                      &semaphore_info,
                                      NULL,
                                      &renderer->render_finished_semaphores[i]) == VK_SUCCESS) ||
            !ASSERT(vkCreateFence(renderer->device.handle,
                                  &fence_info,
                                  NULL,
                                  &renderer->in_flight_fences[i]) == VK_SUCCESS)) {
            return false;
        }
    }

    platform_window_register_resize_callback(window, on_window_resize);

    return true;
}

void render_system_shutdown(struct renderer *renderer) {
    vkDeviceWaitIdle(renderer->device.handle);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(renderer->device.handle, renderer->in_flight_fences[i], NULL);
        vkDestroySemaphore(renderer->device.handle, renderer->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(renderer->device.handle, renderer->image_available_semaphores[i], NULL);
    }
    commandpool_destroy(&renderer->device, &renderer->commandpool);
    swapchain_framebuffers_destroy(&renderer->device, &renderer->swapchain);
    renderpass_destroy(&renderer->device, &renderer->renderpass);
    swapchain_destroy(&renderer->device, &renderer->swapchain);
    device_destroy(&renderer->device);
    vkDestroySurfaceKHR(renderer->instance.handle, renderer->surface, NULL);
    instance_destroy(&renderer->instance);
}

b8 render_system_start_frame(const struct se_window *window, struct renderer *renderer) {
    vkWaitForFences(renderer->device.handle,
                    1,
                    &renderer->in_flight_fences[renderer->current_frame],
                    VK_TRUE,
                    UINT64_MAX);

    VkResult result =
        vkAcquireNextImageKHR(renderer->device.handle,
                              renderer->swapchain.handle,
                              UINT64_MAX,
                              renderer->image_available_semaphores[renderer->current_frame],
                              VK_NULL_HANDLE,
                              &renderer->image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain_recreate(&renderer->device,
                           renderer->surface,
                           window,
                           &renderer->renderpass,
                           &renderer->swapchain);
        return true;
    } else if (!ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)) {
        return false;
    }

    vkResetFences(renderer->device.handle, 1, &renderer->in_flight_fences[renderer->current_frame]);

    vkResetCommandBuffer(renderer->commandbuffers[renderer->current_frame].handle, 0);
    if (!commandbuffer_recording_start(&renderer->commandbuffers[renderer->current_frame])) {
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
    vkCmdSetViewport(renderer->commandbuffers[renderer->current_frame].handle, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = renderer->swapchain.extent,
    };
    vkCmdSetScissor(renderer->commandbuffers[renderer->current_frame].handle, 0, 1, &scissor);

    return true;
}

b8 render_system_end_frame(const struct se_window *window, struct renderer *renderer) {
    renderpass_end(&renderer->commandbuffers[renderer->current_frame]);

    if (!commandbuffer_recording_end(&renderer->commandbuffers[renderer->current_frame])) {
        return false;
    }

    VkSemaphore wait_semaphores[] = {renderer->image_available_semaphores[renderer->current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {
        renderer->render_finished_semaphores[renderer->current_frame]};

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = ARRAY_SIZE(wait_semaphores),
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->commandbuffers[renderer->current_frame].handle,
        .signalSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pSignalSemaphores = signal_semaphores,
    };

    if (!ASSERT(vkQueueSubmit(renderer->device.graphics_queue,
                              1,
                              &submit_info,
                              renderer->in_flight_fences[renderer->current_frame]) == VK_SUCCESS)) {
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

    VkResult result = vkQueuePresentKHR(renderer->device.present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        renderer->framebuffer_resized) {
        renderer->framebuffer_resized = false;
        swapchain_recreate(&renderer->device,
                           renderer->surface,
                           window,
                           &renderer->renderpass,
                           &renderer->swapchain);
    } else if (!ASSERT(result == VK_SUCCESS)) {
        return false;
    }

    renderer->current_frame = (renderer->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

void render_system_end_main_loop(const struct renderer *renderer) {
    vkDeviceWaitIdle(renderer->device.handle);
}

void on_window_resize(void) {
    LOG_INFO("Resize Callback :)");
    state_ptr->framebuffer_resized = true;
}
