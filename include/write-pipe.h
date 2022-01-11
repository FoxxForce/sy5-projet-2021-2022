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


void write_reply_ls(int fd);
int write_reply_so_se(int fd, uint64_t id, int std);
int write_reply_tx(int fd, uint64_t id);
int size_reply_ls();
void write_request_cr(int fd, struct commandline *cl, struct timing *time);
#endif // WRITE_PIPE_H
