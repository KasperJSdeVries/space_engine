#include "render_pass.h"

#include "core/defines.h"
#include "renderer/device.h"
#include "renderer/swapchain.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

RenderPass render_pass_new(const Swapchain *swapchain,
                           const DepthBuffer *depth_buffer,
                           VkAttachmentLoadOp color_buffer_load_op,
                           VkAttachmentLoadOp depth_buffer_load_op) {
    RenderPass self = {
        .swapchain = swapchain,
        .depth_buffer = depth_buffer,
    };

    VkAttachmentDescription color_attachment = {
        .format = swapchain->format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = color_buffer_load_op,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = color_buffer_load_op == VK_ATTACHMENT_LOAD_OP_CLEAR
                             ? VK_IMAGE_LAYOUT_UNDEFINED
                             : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription depth_attachment = {
        .format = depth_buffer->format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = depth_buffer_load_op,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = color_buffer_load_op == VK_ATTACHMENT_LOAD_OP_CLEAR
                             ? VK_IMAGE_LAYOUT_UNDEFINED
                             : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference color_attachment_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_attachment_reference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_reference,
        .pDepthStencilAttachment = &depth_attachment_reference,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription attachments[] = {
        color_attachment,
        depth_attachment,
    };

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = ARRAY_SIZE(attachments),
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    vulkan_check(vkCreateRenderPass(swapchain->device->handle,
                                    &render_pass_info,
                                    NULL,
                                    &self.handle),
                 "create render pass");

    return self;
}

void render_pass_destroy(RenderPass *self) {
    if (self->handle != NULL) {
        vkDestroyRenderPass(self->swapchain->device->handle,
                            self->handle,
                            NULL);
        self->handle = NULL;
    }
}
