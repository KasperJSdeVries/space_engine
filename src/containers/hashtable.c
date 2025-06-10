#include "hashtable.h"
#include "core/assert.h"
#include "core/defines.h"

#include <string.h>

u64 hash_string(const void *key, u32 element_count) {
    static const u64 multiplier = 97;

    const char *string = key;

    unsigned const char *us;
    u64 hash = 0;

    for (us = (unsigned const char *)string; *us; us++) {
        hash = hash * multiplier + *us;
    }

    hash %= element_count;

    return hash;
}

u64 hash_u64(const void *key, u32 element_count) {
    const u64 *number = key;
    return *number % element_count;
}

void hashtable_new(u64 element_size,
                   u32 element_count,
                   void *memory,
                   b8 is_pointer_type,
                   hash_function hash_function,
                   hashtable *out_table) {
    ASSERT(memory != NULL && out_table != NULL);
    ASSERT(element_count && element_size);

    out_table->memory = memory;
    out_table->element_count = element_count;
    out_table->element_size = element_size;
    out_table->is_pointer_type = is_pointer_type;
    out_table->hash = hash_function;
    memset(out_table->memory, 0, element_size * element_count);
}

void hashtable_destroy(hashtable *table) {
    if (table) {
        memset(table, 0, sizeof(hashtable));
    }
}

void hashtable_set(hashtable *table, void *key, void *value) {
    ASSERT(table != NULL && key != NULL && value != NULL);
    ASSERT(!table->is_pointer_type);

    u64 hash = table->hash(key, table->element_count);
    memcpy((u8 *)table->memory + (table->element_size * hash),
           value,
           table->element_size);
}

void hashtable_set_ptr(hashtable *table, void *key, void **value) {
    ASSERT(table != NULL && key != NULL);
    ASSERT(table->is_pointer_type);

    u64 hash = table->hash(key, table->element_count);
    ((void **)table->memory)[hash] = value ? *value : 0;
}

void hashtable_get(hashtable *table, void *key, void *out_value) {
    ASSERT(table != NULL && key != NULL && out_value != NULL);
    ASSERT(!table->is_pointer_type);

    u64 hash = table->hash(key, table->element_count);
    memcpy(out_value,
           (u8 *)table->memory + (table->element_size * hash),
           table->element_size);
}

void hashtable_get_ptr(hashtable *table, void *key, void **out_value) {
    ASSERT(table != NULL && key != NULL && out_value != NULL);
    ASSERT(table->is_pointer_type);

    u64 hash = table->hash(key, table->element_count);
    out_value = ((void **)table->memory)[hash];
}
