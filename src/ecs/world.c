#include "world.h"
#include "core/assert.h"
#include "ecs/component_store.h"

#include <string.h>

world world_new(void) {
    return (world){
        .component_stores = darray_new(component_store),
        .next_id = 0,
    };
}

void _world_add_component(world *world,
                          const char *component_name,
                          u64 component_size) {
    darray_push(world->component_stores,
                _component_store_new(component_name, component_size));
}

entity_id world_create_entity(world *world) { return world->next_id++; }

void _entity_add_component(world *world,
                           entity_id entity,
                           const char *type,
                           void *value_ptr) {
    component_store *store = NULL;

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

    _component_store_insert(store, entity, value_ptr);
}
