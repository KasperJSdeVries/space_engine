#ifndef FILE_H
#define FILE_H

#include "core/defines.h"

u8 *file_read(const char *filename, u64 *out_file_size);

#endif // FILE_H
