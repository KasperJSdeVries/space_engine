#ifndef IMAGE_H
#define IMAGE_H

#include "renderer/buffer.h"
#include "renderer/command_pool.h"
#include "renderer/device.h"
#include "renderer/device_memory.h"

typedef struct {
    const Device *device;
    VkImage handle;
    VkExtent2D extent;
    VkFormat format;
    VkImageLayout layout;
} Image;

Image image_new(const Device *device,
                VkExtent2D extent,
                VkFormat format,
                VkImageTiling tiling,
                VkImageUsageFlags usage);
Image image_sampled_new(const Device *device,
                        VkExtent2D extent,
                        VkFormat format);
Image image_move(Image *other);
void image_destroy(Image *self);

DeviceMemory image_allocate_memory(const Image *self,
                                   VkMemoryPropertyFlags properties);
VkMemoryRequirements image_get_memory_requirements(const Image *self);
void image_memory_barrier_insert(VkCommandBuffer command_buffer,
                                 VkImage image,
                                 VkImageSubresourceRange sub_resource_range,
                                 VkAccessFlags src_access_mask,
                                 VkAccessFlags dst_access_mask,
                                 VkImageLayout old_layout,
                                 VkImageLayout new_layout);

void image_transition_layout(Image *self,
                             CommandPool *command_pool,
                             VkImageLayout new_layout);
void image_copy_from(Image *self,
                     CommandPool *command_pool,
                     const Buffer *buffer);

#endif // IMAGE_H
