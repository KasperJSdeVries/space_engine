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
} component_flags;

typedef struct {
    struct node *root;
    struct node *queue;
    const char *component_name;
    void *component_array;
    u32 component_size;
    u32 component_capacity;
    u32 component_count;
    darray(u64) free_slots;
    u32 order;
    component_flags flags;
} component_store;

component_store _component_store_new(const char *component_name,
                                     u64 component_size);

#define component_store_new(type) _component_store_new(#type, sizeof(type))

void _component_store_insert(component_store *store,
                             entity_id key,
                             const void *value_ptr);

#define component_store_insert(store, key, value)                              \
    do {                                                                       \
        typeof(value) __temp_value_copy__ = value;                             \
        _component_store_insert(store, key, &__temp_value_copy__);             \
    } while (0)

void component_store_print(const component_store *store);

#endif // COMPONENT_STORE_H
