#ifndef ENTITY_H
#define ENTITY_H

#include "core/defines.h"

typedef u32 entity_id;

typedef struct {
    entity_id id;
    u32 generation;
} entity;

#endif // ENTITY_H
