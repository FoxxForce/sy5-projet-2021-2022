#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <stdint.h>

struct commandline {
    uint32_t ARGC;
    char **ARGV;
};

#endif // COMMANDLINE_H
