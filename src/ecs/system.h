#ifndef SYSTEM_H
#define SYSTEM_H

#include "core/defines.h"
#include "ecs/entity.h"
#include "ecs/query.h"

#define MAX_REQUIRED_COMPONENTS 8

typedef enum {
    // TODO: add more system flags
    SYSTEM_FLAG_NETWORKED = 1 << 0,
} SystemFlag;

typedef enum {
    SYSTEM_SCHEDULE_STARTUP,
    SYSTEM_SCHEDULE_UPDATE,
} SystemSchedule;

typedef void (*system_run)(void **components);

typedef struct {
    const char *name;
    Query query;
    system_run fn;
    SystemSchedule schedule;
    SystemFlag flags;
} SystemInfo;

#endif // SYSTEM_H
