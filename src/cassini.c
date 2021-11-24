#include "../include/cassini.h" // Peut causer des soucis
#include "stdint.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "../include/timing.h"
#include "../include/timing-text-io.h"
#include "../include/string.h"


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
  char * request_pipe = "/saturnd-request-pipe";
  char * chemin = malloc(sizeof(char)*250);
  strncat(chemin, pipes_directory, strlen(pipes_directory));
  strncat(chemin, request_pipe, strlen(request_pipe));
  int fd = open(chemin, O_WRONLY | O_APPEND);

  struct timing time;
  int length_timing = timing_from_strings(&time, minutes_str, hours_str, daysofweek_str);

  char *command = malloc(sizeof(char)*(length_timing + 2 ));
  char *timing_string = malloc(TIMING_TEXT_MIN_BUFFERSIZE);
  timing_string_from_timing(timing_string, &time);
  //strcpy(command, timing_string);
  switch(operation){
    case CLIENT_REQUEST_LIST_TASKS :
        write(fd,"LS",2);
        break;
    case CLIENT_REQUEST_CREATE_TASK :
        strcpy(command, "CR");
        char *vv = "CR";
        write(fd, vv, strlen(vv));
        write(fd, &time, sizeof(struct timing));
        /*char *hexa_minutes = malloc(sizeof(char)*17);
        char *hexa_hours = malloc(sizeof(char)*9);
        char *hexa_daysofweek = malloc(sizeof(char)*3);*/
        /*sprintf(hexa_minutes, "%016lx", time.minutes );
        sprintf(hexa_hours, "%08x", time.hours );
        sprintf(hexa_daysofweek, "%02x", time.daysofweek );
        strncat(command, hexa_minutes, 17);
        strncat(command, hexa_hours, 9);
        strncat(command, hexa_daysofweek, 3);*/
        //printf("%s, %s, %s\n", hexa_minutes,hexa_hours, hexa_daysofweek);

        //printf("%ld, %ld\n", sizeof(struct timing), sizeof(vv));
        /*char *toto = malloc(200);
          sprintf(toto, "%s,%ld,%c,%c", "CR", time.minutes, time.hours, time.daysofweek);
          write(fd, toto, 200);
          printf("%s\n", toto);*/
        //struct string s = {11, "echo test-1" };
        //write(fd,s.contents ,s.length);
        break;
  }

  return EXIT_SUCCESS;
  
 error:
  if (errno != 0) perror("main");
  free(pipes_directory);
  pipes_directory = NULL;
  return EXIT_FAILURE;
}

