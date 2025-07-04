#include "image.h"

#include "core/logging.h"
#include "renderer/depth_buffer.h"
#include "renderer/device.h"
#include "renderer/device_memory.h"
#include "renderer/single_time_commands.h"
#include "renderer/vulkan.h"

Image image_new(const Device *device,
                VkExtent2D extent,
                VkFormat format,
                VkImageTiling tiling,
                VkImageUsageFlags usage) {
    Image self = {
        .device = device,
        .extent = extent,
        .format = format,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent =
            {
                .width = extent.width,
                .height = extent.height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = self.layout,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };

    vulkan_check(vkCreateImage(device->handle, &image_info, NULL, &self.handle),
                 "create image");

    return self;
}

Image image_sampled_new(const Device *device,
                        VkExtent2D extent,
                        VkFormat format) {
    return image_new(device,
                     extent,
                     format,
                     VK_IMAGE_TILING_OPTIMAL,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                         VK_IMAGE_USAGE_SAMPLED_BIT);
}

Image image_move(Image *other) {
    Image self = {
        .device = other->device,
        .handle = other->handle,
        .layout = other->layout,
        .format = other->format,
        .extent = other->extent,
    };

    other->handle = NULL;

    return self;
}

void image_destroy(Image *self) {
    if (self->handle != NULL) {
        vkDestroyImage(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}

DeviceMemory image_allocate_memory(const Image *self,
                                   VkMemoryPropertyFlags properties) {
    VkMemoryRequirements requirements = image_get_memory_requirements(self);
    DeviceMemory memory = device_memory_new(self->device,
                                            requirements.size,
                                            requirements.memoryTypeBits,
                                            0,
                                            properties);

    vulkan_check(
        vkBindImageMemory(self->device->handle, self->handle, memory.handle, 0),
        "bind image memory");

    return memory;
}

VkMemoryRequirements image_get_memory_requirements(const Image *self) {
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(self->device->handle,
                                 self->handle,
                                 &requirements);
    return requirements;
}

void image_memory_barrier_insert(VkCommandBuffer command_buffer,
                                 VkImage image,
                                 VkImageSubresourceRange sub_resource_range,
                                 VkAccessFlags src_access_mask,
                                 VkAccessFlags dst_access_mask,
                                 VkImageLayout old_layout,
                                 VkImageLayout new_layout) {
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_access_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = sub_resource_range,
    };

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         &barrier);
}

void image_transition_layout(Image *self,
                             CommandPool *command_pool,
                             VkImageLayout new_layout) {
    single_time_commands_submit(command_pool) {
        LOG_TRACE("Transitioning image layout from %d to %d",
                  self->layout,
                  new_layout);

        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = self->layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = self->handle,
            .subresourceRange =
                {
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (depth_buffer_has_stencil_component(self->format)) {
                barrier.subresourceRange.aspectMask |=
                    VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (self->layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (self->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (self->layout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   new_layout ==
                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            LOG_FATAL("unsupported layout transition");
            exit(EXIT_FAILURE);
        }

        vkCmdPipelineBarrier(command_buffer,
                             srcStage,
                             dstStage,
                             0,
                             0,
                             NULL,
                             0,
                             NULL,
                             1,
                             &barrier);
    }

    self->layout = new_layout;
}

void image_copy_from(Image *self,
                     CommandPool *command_pool,
                     const Buffer *buffer) {
    single_time_commands_submit(command_pool) {
        LOG_TRACE("copying image from buffer");
        VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = {self->extent.width, self->extent.height, 1},
        };

        vkCmdCopyBufferToImage(command_buffer,
                               buffer->handle,
                               self->handle,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
    }
}
