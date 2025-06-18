#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

static void test_manual_tick_system_execution(void **state) { (void)state; }

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_manual_tick_system_execution),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
