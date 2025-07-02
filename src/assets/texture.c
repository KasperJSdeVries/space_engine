#include "assets/texture.h"

#include "core/logging.h"
#include "renderer/sampler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture texture_new(const char *filename) {
    LOG_INFO("loading '%s'...", filename);

    int width, height, channels;
    u8 *pixels =
        stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == NULL) {
        LOG_FATAL("failed to load texture image '%s'", filename);
    }

    return (Texture){sampler_config_default(), width, height, channels, pixels};
}

void texture_destroy(Texture *self) {
    stbi_image_free(self->pixels);
    self->pixels = NULL;
}
