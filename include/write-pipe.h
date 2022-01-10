#ifndef WRITE_PIPE_H
#define WRITE_PIPE_H

#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include <endian.h>
#include "timing.h"
#include "timing-text-io.h"
#include "commandline.h"
#include "task.h"
#include <string.h>


void write_timing_in_pipe(int fd, struct timing *time);
void write_commandline_in_pipe(int fd, struct commandline *cl);
void write_reply_ls(int fd);
void write_reply_so_se(int fd, uint64_t id, int std);
void write_reply_tx(int fd, uint64_t id);
#endif // WRITE_PIPE_H
