#include "gltf_parser.h"

#include "assets/file.h"
#include "core/assert.h"
#include "core/logging.h"
#include "json_parser.h"

#include <stdlib.h>
#include <string.h>

darray(GltfAccessor) parse_accessors(const JsonElement *accessors_element);
darray(GltfBuffer) parse_buffers(const JsonElement *buffers);
darray(GltfBufferView) parse_buffer_views(const JsonElement *buffer_views);
darray(GltfCamera) parse_cameras(const JsonElement *cameras);
darray(GltfMesh) parse_meshes(const JsonElement *meshes);
darray(GltfNode) parse_nodes(const JsonElement *nodes);
darray(GltfScene) parse_scenes(const JsonElement *scenes);

Gltf gltf_parse(const char *filename) {
    u64 file_size;
    char *file_contents = (char *)file_read(filename, &file_size);

    JsonElement gltf;
    if (json_parse(file_contents, &gltf) == false) {
        LOG_FATAL("failed to parse json in: '%s'", filename);
        exit(EXIT_FAILURE);
    }

    free(file_contents);

    ASSERT(gltf.type == JSON_OBJECT);

    Gltf result = {0};

    for (u32 i = 0; i < darray_length(gltf.object); i++) {
        JsonMember member = gltf.object[i];

        LOG_TRACE("%s", member.key);

        if (strcmp(member.key, "accessors") == 0) {
            result.accessors = parse_accessors(&member.value);
        } else if (strcmp(member.key, "buffers") == 0) {
            result.buffers = parse_buffers(&member.value);
        } else if (strcmp(member.key, "bufferViews") == 0) {
            result.buffer_views = parse_buffer_views(&member.value);
        } else if (strcmp(member.key, "cameras") == 0) {
            result.cameras = parse_cameras(&member.value);
        } else if (strcmp(member.key, "meshes") == 0) {
            result.meshes = parse_meshes(&member.value);
        } else if (strcmp(member.key, "nodes") == 0) {
            result.nodes = parse_nodes(&member.value);
        } else if (strcmp(member.key, "scene") == 0) {
            ASSERT(member.value.type == JSON_NUMBER);
            result.scene = (u32)member.value.number;
        } else if (strcmp(member.key, "scenes") == 0) {
            result.scenes = parse_scenes(&member.value);
        } else {
            LOG_WARN("Gltf: Unimplemented Key: '%s'", member.key);
        }
    }

    return result;
}

void glb_parse(const char *filename) {
    UNUSED(filename);
    TODO("Implement glb parsing");
}

darray(GltfAccessor) parse_accessors(const JsonElement *accessors_element) {
    ASSERT(accessors_element->type == JSON_ARRAY);

    darray(GltfAccessor) accessors = darray_new(GltfAccessor);

    for (u32 i = 0; i < darray_length(accessors_element->array); i++) {
        JsonElement accessor_element = accessors_element->array[i];
        ASSERT(accessor_element.type == JSON_OBJECT);

        GltfAccessor accessor = {.buffer_view = -1};

        for (u32 j = 0; j < darray_length(accessor_element.object); j++) {
            JsonMember member = accessor_element.object[j];

            if (strcmp(member.key, "bufferView") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                accessor.buffer_view = (i32)member.value.number;
            } else if (strcmp(member.key, "byteOffset") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                accessor.byte_offset = (u64)member.value.number;
            } else if (strcmp(member.key, "componentType") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                accessor.component_type = (GltfComponentType)(u32)member.value.number;
            } else if (strcmp(member.key, "normalized") == 0) {
                ASSERT(member.value.type == JSON_BOOLEAN);
                accessor.normalized = member.value.boolean;
            } else if (strcmp(member.key, "count") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                accessor.count = (u32)member.value.number;
            } else if (strcmp(member.key, "type") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                if (strcmp(member.value.string, "SCALAR") == 0) {
                    accessor.type = ACCESSOR_TYPE_SCALAR;
                } else if (strncmp(member.value.string, "VEC2", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_VEC2;
                } else if (strncmp(member.value.string, "VEC3", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_VEC3;
                } else if (strncmp(member.value.string, "VEC4", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_VEC4;
                } else if (strncmp(member.value.string, "MAT2", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_MAT2;
                } else if (strncmp(member.value.string, "MAT3", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_MAT3;
                } else if (strncmp(member.value.string, "MAT4", darray_length(member.value.string)) == 0) {
                    accessor.type = ACCESSOR_TYPE_MAT4;
                } else {
                    LOG_FATAL("unknown accessor type: '%s'", member.value.string);
                }
            } else if (strcmp(member.key, "max") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                accessor.max = darray_new(f64);
                for (u32 i = 0; i < darray_length(member.value.array); i++) {
                    JsonElement e = member.value.array[i];
                    ASSERT(e.type == JSON_NUMBER);
                    darray_push(accessor.max, e.number);
                }
            } else if (strcmp(member.key, "min") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                accessor.min = darray_new(f64);
                for (u32 i = 0; i < darray_length(member.value.array); i++) {
                    JsonElement e = member.value.array[i];
                    ASSERT(e.type == JSON_NUMBER);
                    darray_push(accessor.min, e.number);
                }
            } else if (strcmp(member.key, "sparse") == 0) {
                ASSERT(member.value.type == JSON_OBJECT);
                for (u32 i = 0; i < darray_length(member.value.object); i++) {
                    JsonMember sparse_member = member.value.object[i];
                    if (strcmp(sparse_member.key, "count") == 0) {
                        ASSERT(sparse_member.value.type == JSON_NUMBER);
                        accessor.sparse.count = (u32)sparse_member.value.number;
                    } else if (strcmp(sparse_member.key, "indices") == 0) {
                        ASSERT(sparse_member.value.type == JSON_ARRAY);

                        accessor.sparse.indices = darray_new(GltfAccessorSparseIndices);

                        for (u32 i = 0; i < darray_length(sparse_member.value.array); i++) {
                            JsonElement e = sparse_member.value.array[i];
                            ASSERT(e.type == JSON_OBJECT);

                            GltfAccessorSparseIndices indices = {0};

                            for (u32 i = 0; i < darray_length(e.object); i++) {
                                JsonMember indices_member = e.object[i];

                                if (strcmp(indices_member.key, "bufferView") == 0) {
                                    ASSERT(indices_member.value.type == JSON_NUMBER);
                                    indices.buffer_view = (u32)indices_member.value.number;
                                } else if (strcmp(indices_member.key, "byteOffset") == 0) {
                                    ASSERT(indices_member.value.type == JSON_NUMBER);
                                    indices.byte_offset = (u64)indices_member.value.number;
                                } else if (strcmp(indices_member.key, "componentType") == 0) {
                                    ASSERT(indices_member.value.type == JSON_NUMBER);
                                    indices.component_type = (GltfComponentType)indices_member.value.number;
                                }
                            }

                            darray_push(accessor.sparse.indices, indices);
                        }
                    } else if (strcmp(sparse_member.key, "values") == 0) {
                        ASSERT(sparse_member.value.type == JSON_ARRAY);

                        accessor.sparse.values = darray_new(GltfAccessorSparseValues);

                        for (u32 i = 0; i < darray_length(sparse_member.value.array); i++) {
                            JsonElement e = sparse_member.value.array[i];
                            ASSERT(e.type == JSON_OBJECT);

                            GltfAccessorSparseValues values = {0};

                            for (u32 i = 0; i < darray_length(e.object); i++) {
                                JsonMember indices_member = e.object[i];

                                if (strcmp(indices_member.key, "bufferView") == 0) {
                                    ASSERT(indices_member.value.type == JSON_NUMBER);
                                    values.buffer_view = (u32)indices_member.value.number;
                                } else if (strcmp(indices_member.key, "byteOffset") == 0) {
                                    ASSERT(indices_member.value.type == JSON_NUMBER);
                                    values.byte_offset = (u64)indices_member.value.number;
                                }
                            }

                            darray_push(accessor.sparse.values, values);
                        }
                    }
                }
            }
        }

        darray_push(accessors, accessor);
    }

    return accessors;
}

darray(GltfBuffer) parse_buffers(const JsonElement *buffers_element) {
    ASSERT(buffers_element->type == JSON_ARRAY);

    darray(GltfBuffer) buffers = darray_new(GltfBuffer);

    for (u32 i = 0; i < darray_length(buffers_element->array); i++) {
        JsonElement buffer = buffers_element->array[i];
        ASSERT(buffer.type == JSON_OBJECT);

        GltfBuffer b = {0};
        for (u32 i = 0; i < darray_length(buffer.object); i++) {
            JsonMember member = buffer.object[i];
            if (strcmp(member.key, "uri") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                b.uri = strdup(member.value.string);
            }
            if (strcmp(member.key, "byteLength") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                b.byte_length = (u64)member.value.number;
            }
            if (strcmp(member.key, "name") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                b.name = strndup(member.value.string, darray_length(member.value.string));
            }
        }

        darray_push(buffers, b);
    }

    return buffers;
}

static GltfMeshPrimitive parse_mesh_primitive(const JsonElement *mesh_primitive_element) {
    ASSERT(mesh_primitive_element->type == JSON_OBJECT);

    GltfMeshPrimitive result = {
        .attributes = darray_new(GltfMeshPrimitiveAttribute),
        .indices = -1,
        .material = -1,
        .mode = MESH_PRIMITIVE_MODE_TRIANGLES,
    };

    for (u32 i = 0; i < darray_length(mesh_primitive_element->object); i++) {
        JsonMember member = mesh_primitive_element->object[i];
        if (strcmp(member.key, "attributes") == 0) {
            ASSERT(member.value.type == JSON_OBJECT);

            for (u32 j = 0; j < darray_length(member.value.object); j++) {
                JsonMember attribute_element = member.value.object[j];

                GltfMeshPrimitiveAttribute attribute;

                if (strcmp(attribute_element.key, "POSITION") == 0) {
                    attribute.type = ATTRIBUTE_POSITION;
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strcmp(attribute_element.key, "NORMAL") == 0) {
                    attribute.type = ATTRIBUTE_NORMAL;
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strcmp(attribute_element.key, "TANGENT") == 0) {
                    attribute.type = ATTRIBUTE_TANGENT;
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strncmp(attribute_element.key, "TEXCOORD_", sizeof("TEXCOORD_") - 1) == 0) {
                    attribute.type = ATTRIBUTE_TEXCOORD;
                    char *attribute_number = strchr(attribute_element.key, '_') + 1;
                    attribute.number = strtoul(attribute_number, NULL, 10);
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strncmp(attribute_element.key, "COLOR_", sizeof("COLOR_") - 1) == 0) {
                    attribute.type = ATTRIBUTE_COLOR;
                    char *attribute_number = strchr(attribute_element.key, '_') + 1;
                    attribute.number = strtoul(attribute_number, NULL, 10);
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strncmp(attribute_element.key, "JOINTS_", sizeof("JOINTS_") - 1) == 0) {
                    attribute.type = ATTRIBUTE_JOINTS;
                    char *attribute_number = strchr(attribute_element.key, '_') + 1;
                    attribute.number = strtoul(attribute_number, NULL, 10);
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else if (strncmp(attribute_element.key, "WEIGHTS_", sizeof("WEIGHTS_") - 1) == 0) {
                    attribute.type = ATTRIBUTE_WEIGHTS;
                    char *attribute_number = strchr(attribute_element.key, '_') + 1;
                    attribute.number = strtoul(attribute_number, NULL, 10);
                    ASSERT(attribute_element.value.type == JSON_NUMBER);
                    attribute.accessor_index = (u32)attribute_element.value.number;
                } else {
                    LOG_ERROR("GLTF: Unsupported Mesh Attribute: '%s'", attribute_element.key);
                    continue;
                }

                darray_push(result.attributes, attribute);
            }
        } else if (strcmp(member.key, "indices") == 0) {
            ASSERT(member.value.type == JSON_NUMBER);
            result.indices = (i32)member.value.number;
        } else if (strcmp(member.key, "material") == 0) {
            ASSERT(member.value.type == JSON_NUMBER);
            result.material = (i32)member.value.number;
        } else if (strcmp(member.key, "mode") == 0) {
            ASSERT(member.value.type == JSON_NUMBER);
            result.mode = (GltfMeshPrimitiveMode)(u32)member.value.number;
        } else if (strcmp(member.key, "targets") == 0) {
            ASSERT(member.value.type == JSON_ARRAY);

            result.targets = darray_new(darray(GltfMeshPrimitiveAttribute));

            for (u32 j = 0; j < darray_length(member.value.array); j++) {
                JsonElement target_element = member.value.array[j];
                ASSERT(target_element.type == JSON_OBJECT);

                darray(GltfMeshPrimitiveAttribute) target = darray_new(GltfMeshPrimitiveAttribute);

                for (u32 k = 0; k < darray_length(target_element.object); k++) {
                    JsonMember attribute_element = target_element.object[k];

                    GltfMeshPrimitiveAttribute attribute;

                    if (strcmp(attribute_element.key, "POSITION") == 0) {
                        attribute.type = ATTRIBUTE_POSITION;
                        ASSERT(attribute_element.value.type == JSON_NUMBER);
                        attribute.accessor_index = (u32)attribute_element.value.number;
                    } else if (strcmp(attribute_element.key, "NORMAL") == 0) {
                        attribute.type = ATTRIBUTE_NORMAL;
                        ASSERT(attribute_element.value.type == JSON_NUMBER);
                        attribute.accessor_index = (u32)attribute_element.value.number;
                    } else if (strcmp(attribute_element.key, "TANGENT") == 0) {
                        attribute.type = ATTRIBUTE_TANGENT;
                        ASSERT(attribute_element.value.type == JSON_NUMBER);
                        attribute.accessor_index = (u32)attribute_element.value.number;
                    } else if (strncmp(attribute_element.key, "TEXCOORD_", sizeof("TEXCOORD_") - 1) == 0) {
                        attribute.type = ATTRIBUTE_TEXCOORD;
                        char *attribute_number = strchr(attribute_element.key, '_') + 1;
                        attribute.number = strtoul(attribute_number, NULL, 10);
                        ASSERT(attribute_element.value.type == JSON_NUMBER);
                        attribute.accessor_index = (u32)attribute_element.value.number;
                    } else if (strncmp(attribute_element.key, "COLOR_", sizeof("COLOR_") - 1) == 0) {
                        attribute.type = ATTRIBUTE_COLOR;
                        char *attribute_number = strchr(attribute_element.key, '_') + 1;
                        attribute.number = strtoul(attribute_number, NULL, 10);
                        ASSERT(attribute_element.value.type == JSON_NUMBER);
                        attribute.accessor_index = (u32)attribute_element.value.number;
                    } else {
                        LOG_ERROR("GLTF: Unsupported Mesh Morph Target: '%s'", attribute_element.key);
                        continue;
                    }

                    darray_push(target, attribute);
                }

                darray_push(result.targets, target);
            }
        }
    }

    return result;
}

darray(GltfBufferView) parse_buffer_views(const JsonElement *buffer_views_element) {
    ASSERT(buffer_views_element->type == JSON_ARRAY);

    darray(GltfBufferView) buffer_views = darray_new(GltfBufferView);

    for (u32 i = 0; i < darray_length(buffer_views_element->array); i++) {
        JsonElement buffer_view_element = buffer_views_element->array[i];
        ASSERT(buffer_view_element.type == JSON_OBJECT);

        GltfBufferView buffer_view = {.byte_stride = 4};

        for (u32 j = 0; j < darray_length(buffer_view_element.object); j++) {
            JsonMember member = buffer_view_element.object[j];

            if (strcmp(member.key, "buffer") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                buffer_view.buffer = (u32)member.value.number;
            } else if (strcmp(member.key, "byteOffset") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                buffer_view.byte_offset = (u64)member.value.number;
            } else if (strcmp(member.key, "byteLength") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                buffer_view.byte_length = (u64)member.value.number;
            } else if (strcmp(member.key, "byteStride") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                buffer_view.byte_stride = (i32)member.value.number;
            } else if (strcmp(member.key, "target") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                buffer_view.target = (GltfBufferType)(u32)member.value.number;
            } else if (strcmp(member.key, "name") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                buffer_view.name = strndup(member.value.string, darray_length(member.value.string));
            }
        }

        darray_push(buffer_views, buffer_view);
    }

    return buffer_views;
}

darray(GltfCamera) parse_cameras(const JsonElement *cameras_element) {
    ASSERT(cameras_element->type == JSON_ARRAY);

    darray(GltfCamera) cameras = darray_new(GltfCamera);

    for (u32 i = 0; i < darray_length(cameras_element->array); i++) {
        JsonElement camera_element = cameras_element->array[i];
        ASSERT(camera_element.type == JSON_OBJECT);

        GltfCamera camera = {0};

        for (u32 j = 0; j < darray_length(camera_element.object); j++) {
            JsonMember camera_member = camera_element.object[j];

            if (strcmp(camera_member.key, "orthographic") == 0) {
                ASSERT(camera_member.value.type == JSON_OBJECT);
                ASSERT(camera.type == CAMERA_TYPE_UNKNOWN);
                for (u32 k = 0; k < darray_length(camera_member.value.object); k++) {
                    JsonMember member = camera_member.value.object[k];

                    if (strcmp(member.key, "xmag") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.orthographic.xmag = (f32)member.value.number;
                    } else if (strcmp(member.key, "ymag") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.orthographic.ymag = (f32)member.value.number;
                    } else if (strcmp(member.key, "zfar") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.orthographic.zfar = (f32)member.value.number;
                    } else if (strcmp(member.key, "znear") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.orthographic.znear = (f32)member.value.number;
                    }
                }
                camera.type = CAMERA_TYPE_ORTHOGRAPHIC;
            } else if (strcmp(camera_member.key, "perspective") == 0) {
                ASSERT(camera_member.value.type == JSON_OBJECT);
                ASSERT(camera.type == CAMERA_TYPE_UNKNOWN);
                for (u32 k = 0; k < darray_length(camera_member.value.object); k++) {
                    JsonMember member = camera_member.value.object[k];

                    if (strcmp(member.key, "aspectRatio") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.perspective.aspect_ratio = (f32)member.value.number;
                    } else if (strcmp(member.key, "yfov") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.perspective.yfov = (f32)member.value.number;
                    } else if (strcmp(member.key, "zfar") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.perspective.zfar = (f32)member.value.number;
                    } else if (strcmp(member.key, "znear") == 0) {
                        ASSERT(member.value.type == JSON_NUMBER);
                        camera.perspective.znear = (f32)member.value.number;
                    }
                }
                camera.type = CAMERA_TYPE_PERSPECTIVE;
            } else if (strcmp(camera_member.key, "type") == 0) {
                ASSERT(camera_member.value.type == JSON_STRING);
                if (strcmp(camera_member.value.string, "perspective") == 0) {
                    ASSERT(camera.type != CAMERA_TYPE_ORTHOGRAPHIC);
                    camera.type = CAMERA_TYPE_PERSPECTIVE;
                } else if (strcmp(camera_member.value.string, "orthographic") == 0) {
                    ASSERT(camera.type != CAMERA_TYPE_PERSPECTIVE);
                    camera.type = CAMERA_TYPE_ORTHOGRAPHIC;
                } else {
                    LOG_ERROR("GLTF: Unknown camera type: '%s'", camera_member.value.string);
                    continue;
                }
            } else if (strcmp(camera_member.key, "name") == 0) {
                ASSERT(camera_member.value.type == JSON_STRING);
                camera.name = strndup(camera_member.value.string, darray_length(camera_member.value.string));
            }
        }

        darray_push(cameras, camera);
    }
    return cameras;
}

darray(GltfMesh) parse_meshes(const JsonElement *meshes_element) {
    ASSERT(meshes_element->type == JSON_ARRAY);

    darray(GltfMesh) meshes = darray_new(GltfMesh);

    for (u32 i = 0; i < darray_length(meshes_element->array); i++) {
        JsonElement mesh_element = meshes_element->array[i];
        ASSERT(mesh_element.type == JSON_OBJECT);

        GltfMesh mesh = {0};

        for (u32 j = 0; j < darray_length(mesh_element.object); j++) {
            JsonMember member = mesh_element.object[j];
            if (strcmp(member.key, "primitives") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                mesh.primitives = darray_new(GltfMeshPrimitive);

                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    darray_push(mesh.primitives, parse_mesh_primitive(&member.value.array[k]));
                }
            } else if (strcmp(member.key, "weights") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                mesh.weights = darray_new(f32);

                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    darray_push(mesh.weights, (f32)member.value.array[k].number);
                }
            } else if (strcmp(member.key, "name") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                mesh.name = strndup(member.value.string, darray_length(member.value.string));
            }

            darray_push(meshes, mesh);
        }
    }

    return meshes;
}

darray(GltfNode) parse_nodes(const JsonElement *nodes_element) {
    ASSERT(nodes_element->type == JSON_ARRAY);

    darray(GltfNode) nodes = darray_new(GltfNode);

    for (u32 i = 0; i < darray_length(nodes_element->array); i++) {
        JsonElement node = nodes_element->array[i];
        ASSERT(node.type == JSON_OBJECT);

        GltfNode n = {
            -1,
            NULL,
            -1,
            mat4_identity(),
            -1,
            quat_init(0, 0, 0, 1),
            vec3_one(),
            vec3_zero(),
            NULL,
            NULL,
        };

        for (u32 j = 0; j < darray_length(node.object); j++) {
            JsonMember member = node.object[j];
            if (strcmp(member.key, "camera") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                n.camera = (i32)member.value.number;
            } else if (strcmp(member.key, "children") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                n.children = darray_new(u32);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    darray_push(n.children, (u32)member.value.array[k].number);
                }
            } else if (strcmp(member.key, "skin") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                n.skin = (i32)member.value.number;
            } else if (strcmp(member.key, "matrix") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                ASSERT(darray_length(member.value.array) == 16);

                f32 array[16];
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    array[k] = (f32)member.value.array[k].number;
                }
                glm_mat4_make(array, n.matrix.raw);
            } else if (strcmp(member.key, "mesh") == 0) {
                ASSERT(member.value.type == JSON_NUMBER);
                n.mesh = (i32)member.value.number;
            } else if (strcmp(member.key, "rotation") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                ASSERT(darray_length(member.value.array) == 4);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    n.rotation.raw[k] = (f32)member.value.array[k].number;
                }
            } else if (strcmp(member.key, "scale") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                ASSERT(darray_length(member.value.array) == 3);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    n.scale.raw[k] = (f32)member.value.array[k].number;
                }
            } else if (strcmp(member.key, "translation") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                ASSERT(darray_length(member.value.array) == 3);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    n.translation.raw[k] = (f32)member.value.array[k].number;
                }
            } else if (strcmp(member.key, "weights") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                n.weights = darray_new(f32);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    darray_push(n.weights, (f32)member.value.array[k].number);
                }
            } else if (strcmp(member.key, "name") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                n.name = strndup(member.value.string, darray_length(member.value.string));
            }
        }

        darray_push(nodes, n);
    }

    return nodes;
}

darray(GltfScene) parse_scenes(const JsonElement *scenes_element) {
    ASSERT(scenes_element->type == JSON_ARRAY);

    darray(GltfScene) scenes = darray_new(GltfScene);

    for (u32 i = 0; i < darray_length(scenes_element->array); i++) {
        JsonElement scene_element = scenes_element->array[i];
        ASSERT(scene_element.type == JSON_OBJECT);

        GltfScene scene = {0};
        for (u32 j = 0; j < darray_length(scene_element.object); j++) {
            JsonMember member = scene_element.object[j];

            if (strcmp(member.key, "nodes") == 0) {
                ASSERT(member.value.type == JSON_ARRAY);
                scene.nodes = darray_new(u32);
                for (u32 k = 0; k < darray_length(member.value.array); k++) {
                    ASSERT(member.value.array[k].type == JSON_NUMBER);
                    darray_push(scene.nodes, (u32)member.value.array[k].number);
                }
            } else if (strcmp(member.key, "name") == 0) {
                ASSERT(member.value.type == JSON_STRING);
                scene.name = strndup(member.value.string, darray_length(member.value.string));
            }
        }

        darray_push(scenes, scene);
    }

    return scenes;
}
