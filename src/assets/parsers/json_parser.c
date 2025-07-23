#include "json_parser.h"

#include "containers/darray.h"
#include "core/assert.h"
#include "core/logging.h"

#include <ctype.h>
#include <string.h>

static const char *skip_whitespace(const char *str);
static const char *assert_character(const char *str, char c);
static const char *parse_element(const char *string, JsonElement *out_element);
static const char *parse_object(const char *string, JsonElement *out_element);
static const char *parse_array(const char *string, JsonElement *out_element);
static const char *parse_string(const char *string, JsonElement *out_element);
static const char *parse_number(const char *string, JsonElement *out_element);
static const char *parse_boolean(const char *string, JsonElement *out_element);
static const char *parse_null(const char *string, JsonElement *out_element);

b8 json_parse(const char *string, JsonElement *out_element) {
    ASSERT(out_element);
    ASSERT(string);

    return parse_element(string, out_element) != NULL;
}

void json_destroy(JsonElement *element) {
    switch (element->type) {
    case JSON_ARRAY:
        for (u32 i = 0; i < darray_length(element->array); i++) {
            json_destroy(&element->array[i]);
        }
        darray_destroy(element->array);
        break;
    case JSON_OBJECT:
        for (u32 i = 0; i < darray_length(element->object); i++) {
            free(element->object[i].key);
            json_destroy(&element->object[i].value);
        }
        darray_destroy(element->object);
        break;
    case JSON_STRING:
        darray_destroy(element->string);
        break;
    default:
        break;
    }
}

static const char *skip_whitespace(const char *str) {
    while (isspace(*str)) {
        ++str;
    }
    return str;
}

static const char *assert_character(const char *str, char c) {
    if (*str != c) {
        LOG_ERROR("JSON: Expected '%c' but found '%c' at '%.12s'", c, *str, str);
        return NULL;
    }
    ++str;
    return str;
}

static const char *parse_element(const char *string, JsonElement *out_element) {
    string = skip_whitespace(string);

    switch (*string) {
    case '{':
        string = parse_object(string, out_element);
        break;

    case '[':
        string = parse_array(string, out_element);
        break;

    case '"':
        string = parse_string(string, out_element);
        break;

    case 't':
    case 'f':
        string = parse_boolean(string, out_element);
        break;

    case 'n':
        string = parse_null(string, out_element);
        break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        string = parse_number(string, out_element);
        break;

    default:
        LOG_ERROR("Unexpected character '%c' at '%.12s'", *string, string);
        return NULL;
    }

    if (string == NULL) {
        return NULL;
    }

    return skip_whitespace(string);
}

static const char *parse_object(const char *string, JsonElement *out_element) {
    if ((string = assert_character(string, '{')) == NULL) {
        return NULL;
    }

    (*out_element).type = JSON_OBJECT;
    (*out_element).object = darray_new(JsonMember);

    string = skip_whitespace(string);

    b8 first_element = true;

    while (*string != '\0' && *string != '}') {
        if (first_element) {
            first_element = false;
        } else {
            if ((string = assert_character(string, ',')) == NULL) {
                return NULL;
            }
        }

        string = skip_whitespace(string);
        JsonElement key_element = {0};
        if ((string = parse_string(string, &key_element)) == NULL) {
            return NULL;
        }
        string = skip_whitespace(string);

        if ((string = assert_character(string, ':')) == NULL) {
            return NULL;
        }

        JsonElement value = {0};
        if ((string = parse_element(string, &value)) == NULL) {
            return NULL;
        }

        char *key = strndup(key_element.string, darray_length(key_element.string));
        darray_destroy(key_element.string);

        JsonMember member = {key, value};
        darray_push((*out_element).object, member);
    }

    if ((string = assert_character(string, '}')) == NULL) {
        return NULL;
    }

    return string;
}

static const char *parse_array(const char *string, JsonElement *out_element) {
    if ((string = assert_character(string, '[')) == NULL) {
        return NULL;
    }

    (*out_element).type = JSON_ARRAY;
    (*out_element).array = darray_new(JsonElement);

    string = skip_whitespace(string);

    b8 first_element = true;
    while (*string != '\0' && *string != ']') {
        if (first_element) {
            first_element = false;
        } else {
            if ((string = assert_character(string, ',')) == NULL) {
                return NULL;
            }
        }

        JsonElement element = {0};
        if ((string = parse_element(string, &element)) == NULL) {
            return NULL;
        }

        darray_push((*out_element).array, element);
    }

    if ((string = assert_character(string, ']')) == NULL) {
        return NULL;
    }

    return string;
}

static const char *parse_string(const char *string, JsonElement *out_element) {
    if ((string = assert_character(string, '"')) == NULL) {
        return NULL;
    }

    (*out_element).type = JSON_STRING;
    (*out_element).string = darray_new(char);

    while (*string != '\0' && *string != '"') {
        if (*string == '\\') {
            ++string;
            switch (*string) {
            case '"':
                darray_push((*out_element).string, '"');
                break;
            case '\\':
                darray_push((*out_element).string, '\\');
                break;
            case '/':
                darray_push((*out_element).string, '/');
                break;
            case 'b':
                darray_pop((*out_element).string, NULL);
                break;
            case 'f':
                darray_push((*out_element).string, '\f');
                break;
            case 'n':
                darray_push((*out_element).string, '\n');
                break;
            case 'r':
                darray_push((*out_element).string, '\r');
                break;
            case 't':
                darray_push((*out_element).string, '\t');
                break;
            case 'u':
                LOG_ERROR("JSON: we currently don't support unicode input, at "
                          "'%.12s'",
                          string);
                return NULL;
            default:
                LOG_ERROR("Unexpected character '%c' at '%.12s'", *string, string);
                return NULL;
            }
            string++;
        } else {
            if ((*string & 0x80) != 0x80) {
                darray_push((*out_element).string, *string);
                string++;
            } else {
                LOG_ERROR("JSON: we currently don't support unicode input, at "
                          "'%.12s'",
                          string);
                return NULL;
            }
        }
    }

    if ((string = assert_character(string, '"')) == NULL) {
        return NULL;
    }

    return string;
}

static const char *parse_number(const char *string, JsonElement *out_element) {
    (*out_element).type = JSON_NUMBER;

    f64 sign = 1.0;
    f64 value = 0.0;
    if (*string == '-') {
        sign = -1.0;
        ++string;
    }
    if (*string == '0') {
        ++string;
    } else {
        while (*string != '\0' && isdigit(*string)) {
            u32 val = *string - '0';
            value *= 10.0;
            value += val;
            ++string;
        }
    }
    value *= sign;

    if (*string == '.') {
        string++;

        f64 fraction = 0.1;
        while (*string != '\0' && isdigit(*string)) {
            u32 val = *string - '0';
            value += val * fraction;
            fraction *= 0.1;
            ++string;
        }
    }

    if (*string == 'e' || *string == 'E') {
        string++;

        f64 exponent_sign = 1;
        if (*string == '+') {
            string++;
        } else if (*string == '-') {
            exponent_sign = -1;
            string++;
        }

        f64 exponent = 0.0;
        while (*string != '\0' && isdigit(*string)) {
            u32 val = *string - '0';
            exponent *= 10.0;
            exponent += val;
            ++string;
        }

        exponent *= exponent_sign;
        value = value * pow(10.0, exponent);
    }

    (*out_element).number = value;

    return string;
}

static const char *parse_boolean(const char *string, JsonElement *out_element) {
    if (strncmp(string, "true", 4) == 0) {
        (*out_element).type = JSON_BOOLEAN;
        (*out_element).boolean = true;
        return string + 4;
    } else if (strncmp(string, "false", 5) == 0) {
        (*out_element).type = JSON_BOOLEAN;
        (*out_element).boolean = false;
        return string + 5;
    } else {
        LOG_ERROR("JSON: expected 'true' or 'false' at %.12s", string);
        return NULL;
    }
}

static const char *parse_null(const char *string, JsonElement *out_element) {
    if (strncmp(string, "null", 4) == 0) {
        (*out_element).type = JSON_NULL;
        return string + 4;
    } else {
        LOG_ERROR("JSON: expected 'null' at %.12s", string);
        return NULL;
    }
}
