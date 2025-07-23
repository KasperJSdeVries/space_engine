#include "ecs/entity.h"
#include "ecs/world.h"

#include <setjmp.h> // IWYU pragma: keep
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

typedef struct {
    int health;
} Health;

static void test_component_attach_and_retrieve(void **state) {
    (void)state;
    World world = world_new();
    world_register_component(&world, Health);

    entity_id e = world_create_entity(&world);
    Health h = {100};
    world_attach_component(&world, e, Health, h);

    Health *got = world_get_component(&world, e, Health);
    assert_non_null(got);
    assert_int_equal(got->health, 100);
    world_destroy(&world);
}

static void test_detach_component(void **state) {
    (void)state;
    World world = world_new();
    world_register_component(&world, Health);

    entity_id e = world_create_entity(&world);
    Health h = {75};
    world_attach_component(&world, e, Health, h);
    world_detach_component(&world, e, Health);

    void *got = world_get_component(&world, e, Health);
    assert_null(got);
    world_destroy(&world);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_component_attach_and_retrieve),
        cmocka_unit_test(test_detach_component),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
