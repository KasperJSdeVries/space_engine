#ifndef GLTF_PARSER_H
#define GLTF_PARSER_H

#include "containers/darray.h"

typedef enum {
    COMPONENT_TYPE_BYTE = 5120,
    COMPONENT_TYPE_UNSIGNED_BYTE = 5121,
    COMPONENT_TYPE_SHORT = 5122,
    COMPONENT_TYPE_UNSIGNED_SHORT = 5123,
    COMPONENT_TYPE_UNSIGNED_INT = 5125,
    COMPONENT_TYPE_FLOAT = 5126,
} GltfComponentType;

typedef enum {
    ACCESSOR_TYPE_SCALAR,
    ACCESSOR_TYPE_VEC2,
    ACCESSOR_TYPE_VEC3,
    ACCESSOR_TYPE_VEC4,
    ACCESSOR_TYPE_MAT2,
    ACCESSOR_TYPE_MAT3,
    ACCESSOR_TYPE_MAT4,
} GltfAccessorType;

typedef struct {
    u32 buffer_view;
    u64 byte_offset;
    GltfComponentType component_type;
} GltfAccessorSparseIndices;

typedef struct {
    u32 buffer_view;
    u64 byte_offset;
} GltfAccessorSparseValues;

typedef struct {
    u32 count;
    darray(GltfAccessorSparseIndices) indices;
    darray(GltfAccessorSparseValues) values;
} GltfAccessorSparse;

typedef struct {
    i32 buffer_view;
    u64 byte_offset;
    GltfComponentType component_type;
    b8 normalized;
    u32 count;
    GltfAccessorType type;
    darray(f64) max;
    darray(f64) min;
    GltfAccessorSparse sparse;
    char *name;
} GltfAccessor;

typedef struct {
    char *uri;
    u64 byte_length;
    char *name;
} GltfBuffer;

typedef enum {
    BUFFER_TYPE_UNDEFINED = 0,
    BUFFER_TYPE_ARRAY = 34962,
    BUFFER_TYPE_ELEMENT_ARRAY = 34963,
} GltfBufferType;

typedef struct {
    u32 buffer;
    u64 byte_offset;
    u64 byte_length;
    u32 byte_stride;
    GltfBufferType target;
    char *name;
} GltfBufferView;

typedef enum {
    CAMERA_TYPE_UNKNOWN,
    CAMERA_TYPE_PERSPECTIVE,
    CAMERA_TYPE_ORTHOGRAPHIC,
} GltfCameraType;

typedef struct {
    GltfCameraType type;
    union {
        struct {
            f32 xmag;
            f32 ymag;
            f32 zfar;
            f32 znear;
        } orthographic;
        struct {
            f32 aspect_ratio;
            f32 yfov;
            f32 zfar;
            f32 znear;
        } perspective;
    };
    char *name;
} GltfCamera;

typedef struct {
    u32 index;
    u32 tex_coord;
} GltfTextureInfo;

typedef enum {
    ALPHA_MODE_OPAQUE,
    ALPHA_MODE_MASK,
    ALPHA_MODE_BLEND,
} GltfMaterialAlphaMode;

typedef struct {
    char *name;
    struct {
        vec4s base_color_factor;
        GltfTextureInfo base_color_texture;
        f32 metallic_factor;
        f32 roughness_factor;
        GltfTextureInfo metallic_roughness_texture;
    } pbr_metallic_roughness;
    struct {
        u32 index;
        u32 tex_coord;
        f32 scale;
    } normal_texture;
    struct {
        u32 index;
        u32 tex_coord;
        f32 strength;
    } occlusion_texture;
    GltfTextureInfo emissive_texture;
    vec3s emissive_factor;
    GltfMaterialAlphaMode alpha_mode;
    f32 alpha_cutoff;
    b8 double_sided;
} GltfMaterial;

typedef enum {
    MESH_PRIMITIVE_MODE_POINTS = 0,
    MESH_PRIMITIVE_MODE_LINES = 1,
    MESH_PRIMITIVE_MODE_LINE_LOOP = 2,
    MESH_PRIMITIVE_MODE_LINE_STRIP = 3,
    MESH_PRIMITIVE_MODE_TRIANGLES = 4,
    MESH_PRIMITIVE_MODE_TRIANGLE_STRIP = 5,
    MESH_PRIMITIVE_MODE_TRIANGLE_FAN = 6,
} GltfMeshPrimitiveMode;

typedef enum {
    ATTRIBUTE_POSITION,
    ATTRIBUTE_NORMAL,
    ATTRIBUTE_TANGENT,
    ATTRIBUTE_TEXCOORD,
    ATTRIBUTE_COLOR,
    ATTRIBUTE_JOINTS,
    ATTRIBUTE_WEIGHTS,
} GltfMeshPrimitiveAttributeType;

typedef struct {
    GltfMeshPrimitiveAttributeType type;
    u32 number;
    u32 accessor_index;
} GltfMeshPrimitiveAttribute;

typedef struct {
    darray(GltfMeshPrimitiveAttribute) attributes;
    i32 indices;
    i32 material;
    GltfMeshPrimitiveMode mode;
    darray(darray(GltfMeshPrimitiveAttribute)) targets;
} GltfMeshPrimitive;

typedef struct {
    darray(GltfMeshPrimitive) primitives;
    darray(f32) weights;
    char *name;
} GltfMesh;

typedef struct {
    i32 camera;
    darray(u32) children;
    i32 skin;
    mat4s matrix;
    i32 mesh;
    versors rotation;
    vec3s scale;
    vec3s translation;
    darray(f32) weights;
    char *name;
} GltfNode;

typedef enum {
    GLTF_FILTER_UNKNOWN = 0,
    GLTF_FILTER_NEAREST = 9728,
    GLTF_FILTER_LINEAR = 9729,
    GLTF_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
    GLTF_FILTER_LINEAR_MIPMAP_NEAREST = 9985,
    GLTF_FILTER_NEAREST_MIPMAP_LINEAR = 9986,
    GLTF_FILTER_LINEAR_MIPMAP_LINEAR = 9987,
} GltfSamplerFilter;

typedef enum {
    GLTF_WRAPPING_CLAMP_TO_EDGE = 33071,
    GLTF_WRAPPING_MIRRORED_REPEAT = 33648,
    GLTF_WRAPPING_REPEAT = 10497,
} GltfSamplerWrapping;

typedef struct {
    GltfSamplerFilter mag_filter;
    GltfSamplerFilter min_filter;
    GltfSamplerWrapping wrap_s;
    GltfSamplerWrapping wrap_t;
    char *name;
} GltfSampler;

typedef struct {
    darray(u32) nodes;
    char *name;
} GltfScene;

typedef struct {
    i32 sampler;
    i32 source;
    char *name;
} GltfTexture;

typedef struct {
    darray(GltfAccessor) accessors;
    darray(GltfBuffer) buffers;
    darray(GltfBufferView) buffer_views;
    darray(GltfCamera) cameras;
    darray(GltfMesh) meshes;
    darray(GltfNode) nodes;
    u32 scene;
    darray(GltfScene) scenes;
} Gltf;

Gltf gltf_parse(const char *filename);
void glb_parse(const char *filename);

#endif // GLTF_PARSER_H
