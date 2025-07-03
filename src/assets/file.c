#include "file.h"

#include "core/assert.h"
#include "core/logging.h"

u8 *file_read(const char *filename, u64 *out_file_size) {
    ASSERT(out_file_size != NULL);

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        LOG_ERROR("failed to open file '%s'", filename);
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);

    u64 file_size = ftell(fp);
    ASSERT_MSG(file_size > 0, "file is empty");

    rewind(fp);

    u8 *buffer = malloc(file_size);

    if (fread(buffer, file_size, 1, fp) != 1) {
        LOG_ERROR("failed to read file '%s'", filename);
        exit(EXIT_FAILURE);
    }

    *out_file_size = file_size;

    return buffer;
}
