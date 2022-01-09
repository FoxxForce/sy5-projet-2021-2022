#ifndef TASK_H
#define TASK_H

#include "../include/cassini.h"
#include <sys/dir.h>
#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include <endian.h>
#include "../include/timing.h"
#include "../include/timing-text-io.h"
#include "../include/string.h"
#include "../include/server-reply.h"
#include "../include/commandline.h"
#include "../include/write-pipe.h"
#include "../include/task.h"
#include <string.h>
#include <time.h>

uint64_t create_tree(struct timing *time, struct commandline *cl);
int remove_task(int task_id);
void task_commandline(uint64_t id, struct commandline *cl);
void task_timing(uint64_t id, struct timing *time);
int nb_task_created();
int nb_task();
int is_remove_task(int task_id);
int exec_task(int id);
int check_task(char *tasks_directory);
int task_executed(uint64_t id);

#endif // TASK_H
