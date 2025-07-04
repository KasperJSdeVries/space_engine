#ifndef BOTTOM_LEVEL_GEOMETRY_H
#define BOTTOM_LEVEL_GEOMETRY_H

#include "assets/scene.h"
#include "containers/darray.h"
#include "vulkan.h"

typedef struct {
    darray(VkAccelerationStructureGeometryKHR) geometry;
    darray(VkAccelerationStructureBuildRangeInfoKHR) build_offset_info;
} BottomLevelGeometry;

BottomLevelGeometry bottom_level_geometry_new(void);
void bottom_level_geometry_destroy(BottomLevelGeometry *self);

void bottom_level_geometry_add_triangle_geometry(BottomLevelGeometry *self,
                                                 const Scene *scene,
                                                 u32 vertex_offset,
                                                 u32 vertex_count,
                                                 u32 index_offset,
                                                 u32 index_count,
                                                 b8 is_opaque);

#endif // BOTTOM_LEVEL_GEOMETRY_H
