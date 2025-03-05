#ifndef SE_SYSTEM_H
#define SE_SYSTEM_H

#include "core/defines.h"

struct system_state;

u64 system_get_state_size(void);
u64 system_startup(struct system_state *state);
u64 system_shutdown(struct system_state *state);

#endif // SE_SYSTEM_H
