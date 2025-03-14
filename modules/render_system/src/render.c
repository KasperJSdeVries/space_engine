#include "render.h"

#include "commandbuffer.h"
#include "device.h"
#include "instance.h"
#include "pipeline.h"
#include "renderpass.h"
#include "swapchain.h"
#include "types.h"

#include "core/assert.h"
#include "core/defines.h"

#include "vulkan/vulkan_core.h"

static struct render_system_state state = {0};
static struct pipeline triangle_pipeline = {0};

void on_window_resize(void);

b8 render_system_startup(struct se_window *window) {
    if (!instance_create(&state.instance)) {
        return false;
    }

    if (!platform_surface_create(window, state.instance.handle, &state.surface)) {
        return false;
    }

    if (!device_create(&state.instance, state.surface, &state.device)) {
        return false;
    }

    if (!swapchain_create(&state.device, state.surface, window, &state.swapchain)) {
        return false;
    }

    if (!renderpass_create(&state.device, &state.swapchain, &state.renderpass)) {
        return false;
    }

    if (!pipeline_create(&state.device, &state.swapchain, &state.renderpass, &triangle_pipeline)) {
        return false;
    }

    if (!swapchain_framebuffers_create(&state.device, &state.renderpass, &state.swapchain)) {
        return false;
    }

    if (!commandpool_create(&state.device, state.surface, &state.commandpool)) {
        return false;
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!commandbuffer_create(&state.device, &state.commandpool, &state.commandbuffers[i])) {
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
        if (!ASSERT(vkCreateSemaphore(state.device.handle,
                                      &semaphore_info,
                                      NULL,
                                      &state.image_available_semaphores[i]) == VK_SUCCESS) ||
            !ASSERT(vkCreateSemaphore(state.device.handle,
                                      &semaphore_info,
                                      NULL,
                                      &state.render_finished_semaphores[i]) == VK_SUCCESS) ||
            !ASSERT(
                vkCreateFence(state.device.handle, &fence_info, NULL, &state.in_flight_fences[i]) ==
                VK_SUCCESS)) {
            return false;
        }
    }

    platform_window_register_resize_callback(window, on_window_resize);

    return true;
}

void render_system_shutdown(void) {
    vkDeviceWaitIdle(state.device.handle);
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(state.device.handle, state.in_flight_fences[i], NULL);
        vkDestroySemaphore(state.device.handle, state.render_finished_semaphores[i], NULL);
        vkDestroySemaphore(state.device.handle, state.image_available_semaphores[i], NULL);
    }
    commandpool_destroy(&state.device, &state.commandpool);
    swapchain_framebuffers_destroy(&state.device, &state.swapchain);
    pipeline_destroy(&state.device, &triangle_pipeline);
    renderpass_destroy(&state.device, &state.renderpass);
    swapchain_destroy(&state.device, &state.swapchain);
    device_destroy(&state.device);
    vkDestroySurfaceKHR(state.instance.handle, state.surface, NULL);
    instance_destroy(&state.instance);
}

b8 render_system_render_frame(const struct se_window *window) {
    vkWaitForFences(state.device.handle,
                    1,
                    &state.in_flight_fences[state.current_frame],
                    VK_TRUE,
                    UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(state.device.handle,
                                            state.swapchain.handle,
                                            UINT64_MAX,
                                            state.image_available_semaphores[state.current_frame],
                                            VK_NULL_HANDLE,
                                            &state.image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain_recreate(&state.device,
                           state.surface,
                           window,
                           &state.renderpass,
                           &state.swapchain);
        return true;
    } else if (!ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)) {
        return false;
    }

    vkResetFences(state.device.handle, 1, &state.in_flight_fences[state.current_frame]);

    vkResetCommandBuffer(state.commandbuffers[state.current_frame].handle, 0);
    if (!commandbuffer_recording_start(&state.commandbuffers[state.current_frame])) {
        return false;
    }

    renderpass_begin(&state.renderpass,
                     &state.commandbuffers[state.current_frame],
                     &state.swapchain,
                     state.image_index);

    vkCmdBindPipeline(state.commandbuffers[state.current_frame].handle,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      triangle_pipeline.handle);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = state.swapchain.extent.width,
        .height = state.swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(state.commandbuffers[state.current_frame].handle, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = state.swapchain.extent,
    };
    vkCmdSetScissor(state.commandbuffers[state.current_frame].handle, 0, 1, &scissor);

    vkCmdDraw(state.commandbuffers[state.current_frame].handle, 3, 1, 0, 0);

    renderpass_end(&state.commandbuffers[state.current_frame]);

    if (!commandbuffer_recording_end(&state.commandbuffers[state.current_frame])) {
        return false;
    }

    VkSemaphore wait_semaphores[] = {state.image_available_semaphores[state.current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {state.render_finished_semaphores[state.current_frame]};

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = ARRAY_SIZE(wait_semaphores),
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &state.commandbuffers[state.current_frame].handle,
        .signalSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pSignalSemaphores = signal_semaphores,
    };

    if (!ASSERT(vkQueueSubmit(state.device.graphics_queue,
                              1,
                              &submit_info,
                              state.in_flight_fences[state.current_frame]) == VK_SUCCESS)) {
        return false;
    }

    VkSwapchainKHR swapchains[] = {state.swapchain.handle};

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = ARRAY_SIZE(swapchains),
        .pSwapchains = swapchains,
        .pImageIndices = &state.image_index,
        .pResults = NULL,
    };

    result = vkQueuePresentKHR(state.device.present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        state.framebuffer_resized) {
        state.framebuffer_resized = false;
        swapchain_recreate(&state.device,
                           state.surface,
                           window,
                           &state.renderpass,
                           &state.swapchain);
    } else if (!ASSERT(result == VK_SUCCESS)) {
        return false;
    }

    state.current_frame = (state.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

void on_window_resize(void) {
    LOG_INFO("Resize Callback :)");
    state.framebuffer_resized = true;
}
