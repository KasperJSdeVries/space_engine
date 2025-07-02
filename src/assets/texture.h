#ifndef TEXTURE_H
#define TEXTURE_H

#include "renderer/sampler.h"

typedef struct {
    SamplerConfig sampler_config;
    u32 width;
    u32 height;
    u32 channels;
    u8 *pixels;
} Texture;

Texture texture_new(const char *filename);
void texture_destroy(Texture *self);

#endif // TEXTURE_H
