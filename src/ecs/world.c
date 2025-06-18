#include "world.h"
#include "containers/darray.h"
#include "core/assert.h"
#include "ecs/component_store.h"
#include "ecs/entity.h"

#include <string.h>

World world_new(void) {
    return (World){
        .component_stores = darray_new(ComponentStore),
        .free_ids = darray_new(entity_id),
        .next_id = 0,
    };
}

void world_destroy(World *world) {
    for (u32 i = 0; i < darray_length(world->component_stores); i++) {
        component_store_destroy(&world->component_stores[i]);
    }
    darray_destroy(world->component_stores);
    darray_destroy(world->free_ids);
}

void _world_register_component(World *world,
                               const char *component_name,
                               u64 component_size) {
    darray_push(world->component_stores,
                component_store_new(component_name, component_size));
}

entity_id world_create_entity(World *world) {
    if (darray_length(world->free_ids) > 0) {
        entity_id entity;
        darray_pop(world->free_ids, &entity);
        return entity;
    }
    return world->next_id++;
}

void world_destroy_entity(World *world, entity_id entity) {
    if (!world_is_valid_entity(world, entity)) {
        return;
    }

    for (u32 i = 0; i < darray_length(world->component_stores); i++) {
        component_store_remove(&world->component_stores[i], entity);
    }

    darray_push(world->free_ids, entity);
}

b8 world_is_valid_entity(World *world, entity_id entity) {
    if (entity >= world->next_id) {
        return false;
    }

    for (u32 i = 0; i < darray_length(world->free_ids); i++) {
        if (world->free_ids[i] == entity) {
            return false;
        }
    }

    return true;
}

void _world_attach_component(World *world,
                             entity_id entity,
                             const char *type,
                             void *value_ptr) {
    ComponentStore *store = NULL;

    for (u32 i = 0; i < darray_length(world->component_stores); i++) {
        if (strcmp(world->component_stores[i].component_name, type) == 0) {
            store = &world->component_stores[i];
            break;
        }
    }

    if (store == NULL) {
        // TODO: maybe create the component_store, but would need size from
        // signature
        ASSERT_UNREACHABLE();
    }

    component_store_insert(store, entity, value_ptr);
}

void _world_detach_component(World *world,
                             entity_id entity,
                             const char *component_name) {
    ComponentStore *store = NULL;

    for (u32 i = 0; i < darray_length(world->component_stores); i++) {
        if (strcmp(world->component_stores[i].component_name, component_name) ==
            0) {
            store = &world->component_stores[i];
            break;
        }
    }

    if (store == NULL) {
        // TODO: maybe create the component_store, but would need size from
        // signature
        ASSERT_UNREACHABLE();
    }

    component_store_remove(store, entity);
}

void *_world_get_component(const World *world,
                           entity_id entity,
                           const char *type) {
    ComponentStore *store = NULL;

    for (u32 i = 0; i < darray_length(world->component_stores); i++) {
        if (strcmp(world->component_stores[i].component_name, type) == 0) {
            store = &world->component_stores[i];
            break;
        }
    }

    if (store == NULL) {
        // TODO: maybe create the component_store, but would need size from
        // signature
        ASSERT_UNREACHABLE();
    }

    return component_store_find(store, entity);
}
