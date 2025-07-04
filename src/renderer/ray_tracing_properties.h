#ifndef RAY_TRACING_PROPERTIES_H
#define RAY_TRACING_PROPERTIES_H

#include "vulkan.h"

typedef struct {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR pipeline_props;
    VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_props;
} RayTracingProperties;

static inline RayTracingProperties ray_tracing_properties_fetch(
    VkPhysicalDevice device) {
    RayTracingProperties self = {0};

    self.acceleration_props.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

    self.pipeline_props.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    self.pipeline_props.pNext = &self.acceleration_props;

    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &self.pipeline_props,
    };

    vkGetPhysicalDeviceProperties2(device, &props);

    return self;
}

#endif // RAY_TRACING_PROPERTIES_H
