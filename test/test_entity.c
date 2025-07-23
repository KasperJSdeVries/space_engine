#include "ecs/entity.h"
#include "ecs/world.h"

#include <setjmp.h> // IWYU pragma: keep
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

static void test_entity_creation(void **state) {
    (void)state;
    World world = world_new();
    entity_id e = world_create_entity(&world);
    assert_true(world_is_valid_entity(&world, e));
    world_destroy(&world);
}

static void test_entity_destruction(void **state) {
    (void)state;
    World world = world_new();
    entity_id e = world_create_entity(&world);
    world_destroy_entity(&world, e);
    assert_false(world_is_valid_entity(&world, e));
    world_destroy(&world);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_entity_creation),
        cmocka_unit_test(test_entity_destruction),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
