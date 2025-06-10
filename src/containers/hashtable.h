#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "core/defines.h"

typedef u64 (*hash_function)(const void *key, u32 element_count);

u64 hash_string(const void *key, u32 element_count);
u64 hash_u64(const void *key, u32 element_count);

typedef struct {
    u64 element_size;
    u32 element_count;
    b8 is_pointer_type;
    hash_function hash;
    void *memory;
} hashtable;

void hashtable_new(u64 element_size,
                   u32 element_count,
                   void *memory,
                   b8 is_pointer_type,
                   hash_function hash_function,
                   hashtable *out_table);

void hashtable_destroy(hashtable *table);

void hashtable_set(hashtable *table, void *key, void *value);
void hashtable_set_ptr(hashtable *table, void *key, void **value);
void hashtable_get(hashtable *table, void *key, void *out_value);
void hashtable_get_ptr(hashtable *table, void *key, void **out_value);

#endif // HASHTABLE_H
