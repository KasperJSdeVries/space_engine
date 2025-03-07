#include "renderpass.h"

#include "core/assert.h"
#include "render_system/types.h"
#include "vulkan/vulkan_core.h"

b8 renderpass_create(const struct device *device,
                     const struct swapchain *swapchain,
                     struct renderpass *renderpass) {
    VkAttachmentDescription color_attachment = {
        .format = swapchain->format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_reference,
    };

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    if (!ASSERT(vkCreateRenderPass(device->handle, &render_pass_info, NULL, &renderpass->handle) ==
                VK_SUCCESS)) {
        return false;
    }
    return true;
}

void renderpass_destroy(const struct device *device, struct renderpass *renderpass) {
    vkDestroyRenderPass(device->handle, renderpass->handle, NULL);
    renderpass->handle = VK_NULL_HANDLE;
}
