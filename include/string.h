#ifndef STRING_H
#define STRING_H

#include <stdint.h>

struct string {
    uint32_t length;
    char *contents;
};

#endif // STRING_H
