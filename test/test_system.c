#include "core/defines.h"
#include "ecs/entity.h"
#include "ecs/query.h"
#include "ecs/system.h"
#include "ecs/world.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

typedef struct {
    int pos;
} Position;

typedef struct {
    int vel;
} Velocity;

static b8 ran = false;

void move_system(void **components) {
    Position *p = components[0];
    Velocity *v = components[1];

    p->pos += v->vel;
    ran = 1;
}

static void test_manual_tick_system_execution(void **state) {
    (void)state;
    ran = false;

    World world = world_new();

    world_register_component(&world, Position);
    world_register_component(&world, Velocity);

    entity_id e = world_create_entity(&world);
    Position p = {3};
    Velocity v = {2};

    world_attach_component(&world, e, Position, p);
    world_attach_component(&world, e, Velocity, v);

    SystemInfo move_system_info = {
        .query = query_new(Position, Velocity),
        .fn = move_system,
        .schedule = SYSTEM_SCHEDULE_STARTUP,
    };

    world_add_system(&world, move_system_info);

    world_run(&world);

    Position *updated = world_get_component(&world, e, Position);

    assert_true(ran);
    assert_non_null(updated);
    assert_int_equal(updated->pos, 5);

    world_destroy(&world);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_manual_tick_system_execution),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
