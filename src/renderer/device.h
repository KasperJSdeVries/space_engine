#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "containers/darray.h"
#include "renderer/surface.h"
#include "vulkan.h"
#include "vulkan/vulkan_core.h"

typedef struct {
    VkResult (*vkCreateAccelerationStructureKHR)(
        VkDevice device,
        const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkAccelerationStructureKHR *pAccelerationStructure);
    void (*vkDestroyAccelerationStructureKHR)(
        VkDevice device,
        VkAccelerationStructureKHR accelerationStructure,
        const VkAllocationCallbacks *pAllocator);
    void (*vkGetAccelerationStructureBuildSizesKHR)(
        VkDevice device,
        VkAccelerationStructureBuildTypeKHR buildType,
        const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
        const u32 *pMaxPrimitiveCounts,
        VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
    void (*vkCmdBuildAccelerationStructuresKHR)(
        VkCommandBuffer commandBuffer,
        u32 infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const
            *ppBuildRangeInfos);
    void (*vkCmdCopyAccelerationStructureKHR)(
        VkCommandBuffer commandBuffer,
        const VkCopyAccelerationStructureInfoKHR *pInfo);
    void (*vkCmdTraceRaysKHR)(
        VkCommandBuffer commandBuffer,
        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
        u32 width,
        u32 height,
        u32 depth);
    VkResult (*vkCreateRayTracingPipelinesKHR)(
        VkDevice device,
        VkDeferredOperationKHR deferredOperation,
        VkPipelineCache pipelineCache,
        u32 createInfoCount,
        const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
        const VkAllocationCallbacks *pAllocator,
        VkPipeline *pPipelines);
    VkResult (*vkGetRayTracingShaderGroupHandlesKHR)(VkDevice device,
                                                     VkPipeline pipeline,
                                                     uint32_t firstGroup,
                                                     uint32_t groupCount,
                                                     size_t dataSize,
                                                     void *pData);
    VkDeviceAddress (*vkGetAccelerationStructureDeviceAddressKHR)(
        VkDevice device,
        const VkAccelerationStructureDeviceAddressInfoKHR *pInfo);
    void (*vkCmdWriteAccelerationStructuresPropertiesKHR)(
        VkCommandBuffer commandBuffer,
        uint32_t accelerationStructureCount,
        const VkAccelerationStructureKHR *pAccelerationStructures,
        VkQueryType queryType,
        VkQueryPool queryPool,
        uint32_t firstQuery);
} DeviceProcedures;

typedef struct {
    VkPhysicalDevice physical_device;
    const Surface *surface;
    VkDevice handle;
    u32 graphics_family_index;
    u32 compute_family_index;
    u32 present_family_index;
    u32 transfer_family_index;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    DeviceProcedures procedures;
} Device;

Device device_new(VkPhysicalDevice physical_device,
                  const Surface *surface,
                  darray(const char *) required_extensions,
                  VkPhysicalDeviceFeatures device_features,
                  const void *next_device_features);
void device_destroy(Device *device);

void device_wait_idle(const Device *device);

#endif // RENDER_DEVICE_H
