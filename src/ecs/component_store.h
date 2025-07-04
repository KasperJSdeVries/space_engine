#ifndef COMPONENT_STORE_H
#define COMPONENT_STORE_H

#include "containers/darray.h"
#include "ecs/entity.h"

typedef struct {
    // TODO: better types
    void *(*deserialize)(void *);
    void *(*serialize)(void *);
} component_serializer;

typedef enum {
    COMPONENT_FLAG_REPLICATED = 1 << 0,
    COMPONENT_FLAG_INTERPOLATE = 1 << 1,
} ComponentFlags;

typedef struct {
    struct node *root;
    const char *component_name;
    darray(u64) free_slots;
    void *component_array;
    u32 component_size;
    u32 component_capacity;
    u32 component_count;
    u32 order;
    ComponentFlags flags;
} ComponentStore;

ComponentStore component_store_new(const char *component_name,
                                   u64 component_size);

void component_store_destroy(ComponentStore *store);

void component_store_insert(ComponentStore *store,
                            entity_id key,
                            const void *value_ptr);

void component_store_remove(ComponentStore *store, entity_id key);

void *component_store_find(const ComponentStore *store, entity_id key);

#endif // COMPONENT_STORE_H
