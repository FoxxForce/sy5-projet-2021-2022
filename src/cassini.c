#include "../include/cassini.h" // Peut causer des soucis
#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "../include/timing.h"
#include "../include/timing-text-io.h"
#include "../include/string.h"
#include "../include/commandline.h"
#include "../include/write-pipe.h"
#include "../include/read-pipe.h"
#include <endian.h>

const char usage_info[] = "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";

int main(int argc, char * argv[]) {
  errno = 0;

  char * minutes_str = "*";
  char * hours_str = "*";
  char * daysofweek_str = "*";
  char * pipes_directory = NULL;

  uint16_t operation = CLIENT_REQUEST_LIST_TASKS;
  uint64_t taskid;

  int opt;
  char * strtoull_endp;
  while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
    switch (opt) {
    case 'm':
      minutes_str = optarg;
      break;
    case 'H':
      hours_str = optarg;
      break;
    case 'd':
      daysofweek_str = optarg;
      break;
    case 'p':
      pipes_directory = strdup(optarg);
      if (pipes_directory == NULL) goto error;
      break;
    case 'l':
      operation = CLIENT_REQUEST_LIST_TASKS;
      break;
    case 'c':
      operation = CLIENT_REQUEST_CREATE_TASK;
      break;
    case 'q':
      operation = CLIENT_REQUEST_TERMINATE;
      break;
    case 'r':
      operation = CLIENT_REQUEST_REMOVE_TASK;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'x':
      operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'o':
      operation = CLIENT_REQUEST_GET_STDOUT;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'e':
      operation = CLIENT_REQUEST_GET_STDERR;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'h':
      printf("%s", usage_info);
      return 0;
    case '?':
      fprintf(stderr, "%s", usage_info);
      goto error;
    }
  }

  // --------
  // | TODO |
  // --------
  if(pipes_directory==NULL){
      char *username = getenv("USER");
      pipes_directory = malloc(500);
      sprintf(pipes_directory, "/tmp/%s/saturnd/pipes", username);
  }
  int attributs;
  
  char * path_request_pipe = malloc(sizeof(char)*1000);
  sprintf(path_request_pipe, "%s/saturnd-request-pipe", pipes_directory);
  int fd = open(path_request_pipe, O_WRONLY | O_APPEND);

  char * path_reply_pipe = malloc(sizeof(char)*1000);
  sprintf(path_reply_pipe, "%s/saturnd-reply-pipe", pipes_directory);
  int fd2 = 0;

  if(fd2==-1 || fd==-1){
      goto error;
  }

  char *reptype = malloc(sizeof(uint16_t));
  uint64_t id;
  switch(operation){
    case CLIENT_REQUEST_LIST_TASKS :
        write(fd, "LS", 2);
        fd2 = open(path_reply_pipe, O_RDONLY);
        read_reply_ls(fd2);
        break;
    case CLIENT_REQUEST_CREATE_TASK :
        write(fd, "CR", 2);
        struct timing time;
        timing_from_strings(&time, minutes_str, hours_str, daysofweek_str);
        write_timing_in_pipe(fd,&time);
        struct commandline cl;
        commandline_from_arguments(&cl, argc, argv);
        write_commandline_in_pipe(fd, &cl);
         //sleep(5);
        fd2 = open(path_reply_pipe, O_RDONLY);
        read(fd2, reptype, sizeof(uint16_t));
        read(fd2, &id, sizeof(uint64_t));
        id = htobe64(id);
        printf("%lu", id);
        free_commandline(&cl);
        break;
    case CLIENT_REQUEST_REMOVE_TASK :
        write(fd, "RM", 2);
        taskid = htobe64(taskid);
        write(fd, &taskid, sizeof(uint64_t));
        fd2 = open(path_reply_pipe, O_RDONLY);
        if(read_reply_rm(fd2)==-1){
            goto error;
        }
        break;
    case CLIENT_REQUEST_GET_STDOUT :
        write(fd,"SO", 2);
        taskid = htobe64(taskid);
        write(fd, &taskid, sizeof(uint64_t));
        fd2 = open(path_reply_pipe, O_RDONLY);
        if(read_reply_so_se(fd2)==-1){
            goto error;
        }
        break;
    case CLIENT_REQUEST_GET_STDERR :
        write(fd,"SE", 2);
        taskid = htobe64(taskid);
        write(fd, &taskid, sizeof(uint64_t));
        fd2 = open(path_reply_pipe, O_RDONLY);
        if(read_reply_so_se(fd2)==-1){
            goto error;
        }
        break;
    case CLIENT_REQUEST_TERMINATE :
        write(fd,"TM", 2);
        break;
    case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
        write(fd,"TX", 2);
        taskid = htobe64(taskid);
        write(fd, &taskid, sizeof(uint64_t));
        fd2 = open(path_reply_pipe, O_RDONLY);
        if(read_reply_tx(fd2)==-1){
            goto error;
        }
        break;
  }
    free(pipes_directory);
    free(path_request_pipe);
    free(path_reply_pipe);
    free(reptype);
    close(fd);
    close(fd2);
  return EXIT_SUCCESS;

 error:
  if (errno != 0) perror("main");
  free(pipes_directory);
  free(path_request_pipe);
  free(path_reply_pipe);
  free(reptype);
  close(fd);
  close(fd2);
  pipes_directory = NULL;
  return EXIT_FAILURE;
}
