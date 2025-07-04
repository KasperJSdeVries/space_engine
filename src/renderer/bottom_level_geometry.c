#include "bottom_level_geometry.h"

#include "assets/vertex.h"
#include "containers/darray.h"
#include "renderer/buffer.h"

BottomLevelGeometry bottom_level_geometry_new(void) {
    return (BottomLevelGeometry){
        darray_new(VkAccelerationStructureGeometryKHR),
        darray_new(VkAccelerationStructureBuildRangeInfoKHR),
    };
}

void bottom_level_geometry_destroy(BottomLevelGeometry *self) {
    if (self->geometry != NULL) {
        darray_destroy(self->geometry);
        darray_destroy(self->build_offset_info);
    }
}

void bottom_level_geometry_add_triangle_geometry(BottomLevelGeometry *self,
                                                 const Scene *scene,
                                                 u32 vertex_offset,
                                                 u32 vertex_count,
                                                 u32 index_offset,
                                                 u32 index_count,
                                                 b8 is_opaque) {
    VkAccelerationStructureGeometryKHR geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry.triangles =
            {
                .sType =
                    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                .vertexData.deviceAddress =
                    buffer_get_device_address(&scene->vertex_buffer),
                .vertexStride = sizeof(Vertex),
                .maxVertex = vertex_count,
                .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
                .indexData.deviceAddress =
                    buffer_get_device_address(&scene->index_buffer),
                .indexType = VK_INDEX_TYPE_UINT32,
            },
        .flags = is_opaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0,
    };

    VkAccelerationStructureBuildRangeInfoKHR build_offset_info = {
        .firstVertex = vertex_offset / sizeof(Vertex),
        .primitiveOffset = index_offset,
        .primitiveCount = index_count / 3,
        .transformOffset = 0,
    };

    darray_push(self->geometry, geometry);
    darray_push(self->build_offset_info, build_offset_info);
}
