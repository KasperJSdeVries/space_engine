#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "containers/darray.h"

typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL,
} JsonElementType;

struct JsonMember;

typedef struct JsonElement {
    JsonElementType type;
    union {
        darray(struct JsonMember) object;
        darray(struct JsonElement) array;
        darray(char) string;
        f64 number;
        b8 boolean;
    };
} JsonElement;

typedef struct JsonMember {
    char *key;
    JsonElement value;
} JsonMember;

b8 json_parse(const char *string, JsonElement *out_element);
void json_destroy(JsonElement *element);

#endif // JSON_PARSER_H
