#ifndef RENDER_COMMANDBUFFER_H
#define RENDER_COMMANDBUFFER_H

#include "types.h"

b8 commandpool_create(const struct device *device,
                      const struct surface *surface,
                      struct commandpool *commandpool);
void commandpool_destroy(const struct device *device,
                         struct commandpool *commandpool);

b8 commandbuffer_create(const struct device *device,
                        const struct commandpool *commandpool,
                        struct commandbuffer *commandbuffer);
b8 commandbuffer_recording_start(const struct commandbuffer *commandbuffer);
b8 commandbuffer_recording_end(const struct commandbuffer *commandbuffer);

#endif // RENDER_COMMANDBUFFER_H
