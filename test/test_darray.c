#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "containers/darray.h"
#include "core/defines.h"

static void test_darray_push(void **state) {
    (void)state;
    darray(u64) da = darray_new(u64);
    darray_push(da, 23);
    assert_int_equal(da[0], 23);
    assert_int_equal(darray_length(da), 1);
    darray_destroy(da);
}

static void test_darray_push_and_pop(void **state) {
    (void)state;
    darray(u64) da = darray_new(u64);
    darray_push(da, 42);
    u64 got;
    darray_pop(da, &got);
    assert_int_equal(darray_length(da), 0);
    assert_int_equal(42, got);
    darray_destroy(da);
}

static void test_darray_insert_at(void **state) {
    (void)state;
    darray(u64) da = darray_new(u64);
    for (u32 i = 0; i < 10; i++) {
        darray_push(da, 0xdeadbeef);
    }
    darray_insert_at(da, 5, 0x42);
    assert_int_equal(darray_length(da), 11);
    assert_int_equal(da[5], 0x42);
    darray_destroy(da);
}

static void test_darray_remove_at(void **state) {
    (void)state;

    darray(u64) da = darray_new(u64);
    for (u32 i = 0; i < 10; i++) {
        darray_push(da, i);
    }

    darray_remove_at(da, 5);

    assert_int_equal(darray_length(da), 9);
    for (u32 i = 0; i < 9; i++) {
        assert_int_not_equal(da[i], 5);
    }
    darray_destroy(da);
}

static void test_darray_remove_at_sorted(void **state) {
    (void)state;

    darray(u64) da = darray_new(u64);
    for (u32 i = 0; i < 10; i++) {
        darray_push(da, i);
    }
    darray_remove_at_sorted(da, 5);
    assert_int_equal(darray_length(da), 9);
    for (u32 i = 0, j = 0; i < 9; i++, j++) {
        if (i == 5) {
            j++;
        }
        assert_int_equal(da[i], j);
    }
    darray_destroy(da);
}

static void test_darray_append(void **state) {
    (void)state;

    darray(u64) array_1 = darray_new(u64);
    darray(u64) array_2 = darray_new(u64);
    for (u64 i = 0; i < 10; i++) {
        darray_push(array_1, i);
    }
    for (u64 i = 10; i < 20; i++) {
        darray_push(array_2, i);
    }
    darray_append(array_1, array_2);

    assert_int_equal(darray_length(array_1), 20);
    for (u32 i = 0; i < 20; i++) {
        assert_int_equal(array_1[i], i);
    }
    darray_destroy(array_1);
    darray_destroy(array_2);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_darray_push),
        cmocka_unit_test(test_darray_push_and_pop),
        cmocka_unit_test(test_darray_insert_at),
        cmocka_unit_test(test_darray_remove_at),
        cmocka_unit_test(test_darray_remove_at_sorted),
        cmocka_unit_test(test_darray_append),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
