#include "model.h"

#include "assets/material.h"
#include "assets/parsers/obj_parser.h"
#include "containers/darray.h"
#include "core/logging.h"

Model model_load(const char *filename) {
    parse_obj(filename);
    return (Model){0};
}
