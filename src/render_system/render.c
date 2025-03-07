#include "render.h"

#include "core/assert.h"
#include "core/defines.h"
#include "render_system/commandbuffer.h"
#include "render_system/device.h"
#include "render_system/instance.h"
#include "render_system/pipeline.h"
#include "render_system/renderpass.h"
#include "render_system/swapchain.h"
#include "render_system/types.h"
#include "vulkan/vulkan_core.h"

static struct render_system_state state = {0};
static struct pipeline triangle_pipeline = {0};

b8 render_system_startup(const struct se_window *window) {
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
    if (!commandbuffer_create(&state.device, &state.commandpool, &state.commandbuffer)) {
        return false;
    }

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    if (!ASSERT(vkCreateSemaphore(state.device.handle,
                                  &semaphore_info,
                                  NULL,
                                  &state.image_available_semaphore) == VK_SUCCESS) ||
        !ASSERT(vkCreateSemaphore(state.device.handle,
                                  &semaphore_info,
                                  NULL,
                                  &state.render_finished_semaphore) == VK_SUCCESS) ||
        !ASSERT(vkCreateFence(state.device.handle, &fence_info, NULL, &state.in_flight_fence) ==
                VK_SUCCESS)) {
        return false;
    }

    return true;
}

void render_system_shutdown(void) {
    vkDeviceWaitIdle(state.device.handle);
    vkDestroyFence(state.device.handle, state.in_flight_fence, NULL);
    vkDestroySemaphore(state.device.handle, state.render_finished_semaphore, NULL);
    vkDestroySemaphore(state.device.handle, state.image_available_semaphore, NULL);
    commandpool_destroy(&state.device, &state.commandpool);
    swapchain_framebuffers_destroy(&state.device, &state.swapchain);
    pipeline_destroy(&state.device, &triangle_pipeline);
    renderpass_destroy(&state.device, &state.renderpass);
    swapchain_destroy(&state.device, &state.swapchain);
    device_destroy(&state.device);
    vkDestroySurfaceKHR(state.instance.handle, state.surface, NULL);
    instance_destroy(&state.instance);
}

b8 render_system_render_frame(void) {
    vkWaitForFences(state.device.handle, 1, &state.in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(state.device.handle, 1, &state.in_flight_fence);

    u32 image_index;
    vkAcquireNextImageKHR(state.device.handle,
                          state.swapchain.handle,
                          UINT64_MAX,
                          state.image_available_semaphore,
                          VK_NULL_HANDLE,
                          &image_index);

    vkResetCommandBuffer(state.commandbuffer.handle, 0);
    if (!commandbuffer_recording_start(&state.commandbuffer)) {
        return false;
    }

    renderpass_begin(&state.renderpass, &state.commandbuffer, &state.swapchain, image_index);

    vkCmdBindPipeline(state.commandbuffer.handle,
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
    vkCmdSetViewport(state.commandbuffer.handle, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = state.swapchain.extent,
    };
    vkCmdSetScissor(state.commandbuffer.handle, 0, 1, &scissor);

    vkCmdDraw(state.commandbuffer.handle, 3, 1, 0, 0);

    renderpass_end(&state.commandbuffer);

    if (!commandbuffer_recording_end(&state.commandbuffer)) {
        return false;
    }

    VkSemaphore wait_semaphores[] = {state.image_available_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {state.render_finished_semaphore};

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = ARRAY_SIZE(wait_semaphores),
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &state.commandbuffer.handle,
        .signalSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pSignalSemaphores = signal_semaphores,
    };

    if (!ASSERT(
            vkQueueSubmit(state.device.graphics_queue, 1, &submit_info, state.in_flight_fence) ==
            VK_SUCCESS)) {
        return false;
    }

    VkSwapchainKHR swapchains[] = {state.swapchain.handle};

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = ARRAY_SIZE(signal_semaphores),
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = ARRAY_SIZE(swapchains),
        .pSwapchains = swapchains,
        .pImageIndices = &image_index,
        .pResults = NULL,
    };

    vkQueuePresentKHR(state.device.present_queue, &present_info);

    return true;
}
