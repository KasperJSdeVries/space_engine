#include "obj_parser.h"

#include "assets/file.h"
#include "core/defines.h"

void parse_obj(const char *filename) {
    u64 buffer_size = 0;
    u8 *buffer = file_read(filename, &buffer_size);

    printf("%.*s\n", (int)buffer_size, buffer);

    free(buffer);
}
