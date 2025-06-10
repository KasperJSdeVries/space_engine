#ifndef ECS_WORLD_H
#define ECS_WORLD_H

#include "component_store.h"
#include "containers/darray.h"
#include "ecs/entity.h"

typedef struct {
    darray(component_store) component_stores;
    entity_id next_id;
} world;

world world_new(void);

void _world_add_component(world *world,
                          const char *component_name,
                          u64 component_size);

#define world_add_component(world, type)                                       \
    _world_add_component(world, #type, sizeof(type));

entity_id world_create_entity(world *world);

void _entity_add_component(world *world,
                           entity_id entity,
                           const char *type,
                           void *value_ptr);

#define entity_add_component(world, entity, type, component)                   \
    do {                                                                       \
        type __temporary_value_copy__ = component;                             \
        _entity_add_component(world,                                           \
                              entity,                                          \
                              #type,                                           \
                              &__temporary_value_copy__);                      \
    } while (0)

#endif // ECS_WORLD_H
