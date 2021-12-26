#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct commandline {
    uint32_t ARGC;
    char **ARGV;
};

int commandline_from_arguments(struct commandline *dest, int argc, char **argv);
void free_commandline(struct commandline *cl);
void print_commandline(struct commandline *cl);
int commandline_size(const struct commandline *cl);
void commandline_string_from_commandline(char * dest, const struct commandline *cl);
#endif // COMMANDLINE_H
