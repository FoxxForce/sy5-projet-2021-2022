#ifndef TASK_H
#define TASK_H

#include <sys/dir.h>
#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include <endian.h>
#include "timing.h"
#include "timing-text-io.h"
#include "server-reply.h"
#include "commandline.h"
#include "write-pipe.h"
#include "client-request.h"
#include "task.h"
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "path.h"

uint64_t create_tree(struct timing *time, struct commandline *cl);
int remove_task(int task_id);
int task_commandline(uint64_t id, struct commandline *cl);
void task_timing(uint64_t id, struct timing *time);
int nb_task_created();
int nb_task();
int is_remove_task(int task_id);
int exec_task(int id);
int check_task(char *tasks_directory);
int task_executed(uint64_t id);
int exitcode_task(int pid_child, uint16_t exitcode);
int kill_childs();
#endif // TASK_H
