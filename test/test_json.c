#include "assets/parsers/json_parser.h"

#include <setjmp.h> // IWYU pragma: keep
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

static void test_json_null(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("null", &result);

    assert_int_equal(JSON_NULL, result.type);

    json_destroy(&result);
}

static void test_json_true(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("true", &result);

    assert_int_equal(JSON_BOOLEAN, result.type);
    assert_int_equal(true, result.boolean);

    json_destroy(&result);
}

static void test_json_false(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("false", &result);

    assert_int_equal(JSON_BOOLEAN, result.type);
    assert_int_equal(false, result.boolean);

    json_destroy(&result);
}

static void test_json_integer(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("12345", &result);

    assert_int_equal(JSON_NUMBER, result.type);
    assert_float_equal(result.number, 12345.0, F64_EPSILON);

    json_destroy(&result);
}

static void test_json_float(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("123.45", &result);

    assert_int_equal(JSON_NUMBER, result.type);
    assert_float_equal(result.number, 123.45, F64_EPSILON);

    json_destroy(&result);
}

static void test_json_exponential(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("1.23e4", &result);

    assert_int_equal(JSON_NUMBER, result.type);
    assert_float_equal(result.number, 12300.0, F64_EPSILON);

    json_destroy(&result);
}

static void test_json_negative_exponential(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("1234e-4", &result);

    assert_int_equal(JSON_NUMBER, result.type);
    assert_float_equal(result.number, 0.1234, F64_EPSILON);

    json_destroy(&result);
}

static void test_json_object(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("{\"key\": \"value\"}", &result);

    assert_int_equal(JSON_OBJECT, result.type);
    assert_true(darray_length(result.object) == 1);
    assert_string_equal(result.object[0].key, "key");
    assert_int_equal(result.object[0].value.type, JSON_STRING);
    assert_string_equal(result.object[0].value.string, "value");

    json_destroy(&result);
}

static void test_json_array(void **state) {
    (void)state;

    JsonElement result = {0};
    json_parse("[1, 2, 3]", &result);

    assert_int_equal(JSON_ARRAY, result.type);
    assert_true(darray_length(result.array) == 3);
    assert_float_equal(result.array[0].number, 1.0, F64_EPSILON);
    assert_float_equal(result.array[1].number, 2.0, F64_EPSILON);
    assert_float_equal(result.array[2].number, 3.0, F64_EPSILON);

    json_destroy(&result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_json_null),
        cmocka_unit_test(test_json_true),
        cmocka_unit_test(test_json_false),
        cmocka_unit_test(test_json_integer),
        cmocka_unit_test(test_json_float),
        cmocka_unit_test(test_json_exponential),
        cmocka_unit_test(test_json_negative_exponential),
        cmocka_unit_test(test_json_object),
        cmocka_unit_test(test_json_array),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
