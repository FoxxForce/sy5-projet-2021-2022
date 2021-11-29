#ifndef READ_PIPE_H
#define READ_PIPE_H

#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include <endian.h>
#include "../include/timing.h"
#include "../include/timing-text-io.h"
#include "../include/string.h"
#include "../include/commandline.h"
#include <string.h>

void read_commandline(int fd, struct commandline *commandTask);
void read_id(int fd);
void read_timing(int fd, struct timing *timingTask, char *timingString);
void read_reply_cr(int fd);

#endif // READ_PIPE_H
