#ifndef ECS_WORLD_H
#define ECS_WORLD_H

#include "component_store.h"
#include "containers/darray.h"
#include "ecs/entity.h"
#include "ecs/system.h"

typedef struct {
    darray(ComponentStore) component_stores;
    darray(entity_id) free_ids;
    entity_id next_id;
} World;

World world_new(void);

void world_destroy(World *world);

void _world_register_component(World *world,
                               const char *component_name,
                               u64 component_size);

#define world_register_component(world, type)                                  \
    _world_register_component(world, #type, sizeof(type));

entity_id world_create_entity(World *world);
void world_destroy_entity(World *world, entity_id entity);
b8 world_is_valid_entity(World *world, entity_id entity);

void _world_attach_component(World *world,
                             entity_id entity,
                             const char *type,
                             void *value_ptr);

#define world_attach_component(world, entity, type, component)                 \
    do {                                                                       \
        type __temporary_value_copy__ = component;                             \
        _world_attach_component(world,                                         \
                                entity,                                        \
                                #type,                                         \
                                &__temporary_value_copy__);                    \
    } while (0)

void _world_detach_component(World *world,
                             entity_id entity,
                             const char *component_name);

#define world_detach_component(world, entity, type)                            \
    do {                                                                       \
        (void)sizeof(type); /* for lsp autocomplete */                         \
        _world_detach_component(world, entity, #type);                         \
    } while (0)

void *_world_get_component(const World *world,
                           entity_id entity,
                           const char *type);

#define world_get_component(world, entity, type)                               \
    (type *)_world_get_component(world, entity, #type)

void world_add_system(World *world, System system);

#endif // ECS_WORLD_H
