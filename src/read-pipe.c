#include "../include/read-pipe.h"
#include "../include/cassini.h"
#include "task.c"
void read_reply_ls(int fd){
    char *reptype = malloc(sizeof(uint16_t));
    read(fd, reptype, sizeof(uint16_t));
    struct timing timingTask;
    char *timingString = malloc(sizeof(struct timing));
    uint32_t nbTasks;
    read(fd, &nbTasks, sizeof(uint32_t));
    nbTasks = htobe32(nbTasks);
    struct commandline commandTask;
    for(int i=0; i<nbTasks; i++){
        read_id(fd);
        read_timing(fd, &timingTask, timingString);
        read_commandline(fd, &commandTask);
        printf("%s ", timingString);
        print_commandline(&commandTask);
    }
    free_commandline(&commandTask);
    free(timingString);
    free(reptype);
}

void read_timing(int fd, struct timing *timingTask, char *timingString){
    read(fd, &timingTask->minutes, sizeof(uint64_t));
    timingTask->minutes = htobe64(timingTask->minutes);
    read(fd, &timingTask->hours, sizeof(uint32_t));
    timingTask->hours = htobe32(timingTask->hours);
    read(fd, &timingTask->daysofweek, sizeof(uint8_t));
    timing_string_from_timing(timingString, timingTask);
}

void read_commandline(int fd, struct commandline *commandTask){
    uint32_t argc;
    read(fd, &argc, sizeof(uint32_t));
    argc = htobe32(argc);
    commandTask->ARGC = argc;

    commandTask->ARGV = malloc(sizeof(char *)*argc);
    memset(commandTask->ARGV, 0, sizeof(char *)*argc);
    uint32_t length;
    for(int i=0; i<commandTask->ARGC; i++){
        read(fd, &length, sizeof(uint32_t));
        length = htobe32(length);
        commandTask->ARGV[i] = malloc(sizeof(char)*(length+1));
        memset(commandTask->ARGV[i], 0, length+1);
        read(fd, commandTask->ARGV[i], length);
        commandTask->ARGV[i][length] = '\0';
    }
}

void read_id(int fd){
    uint64_t id;
    read(fd, &id, sizeof(uint64_t));
    id = htobe64(id);
    printf("%ld: ", id);
}

int read_reply_tx(int fd){
    uint16_t reptype;
    read(fd, &reptype, sizeof(uint16_t));
    reptype = htobe16(reptype);
    if(reptype==SERVER_REPLY_ERROR){
        uint16_t error;
        read(fd, &error, sizeof(uint16_t));
        printf("%d", htobe16(error));
        return -1;
    }
    uint64_t secondes;
    uint32_t  nb_runs;
    uint16_t exitcode;
    read(fd, &nb_runs, sizeof(uint32_t));
    nb_runs = htobe32(nb_runs);
    struct tm  *date;
    for(int i=0; i<nb_runs; i++){
        read(fd, &secondes, sizeof(uint64_t));
        read(fd, &exitcode, sizeof(uint16_t));
        secondes = htobe64(secondes);
        exitcode = htobe16(exitcode);
        date = localtime(&secondes);

        printf("%04d-%02d-%02d %02d:%02d:%02d %d\n",date->tm_year+1900, date->tm_mon+1,
               date->tm_mday, date->tm_hour,date->tm_min, date->tm_sec, exitcode);
    }
    return 0;
}

int read_reply_so_se(int fd) {
    uint16_t reptype;
    read(fd, &reptype, sizeof(uint16_t));
    reptype = htobe16(reptype);
    if (reptype == SERVER_REPLY_ERROR) {
        uint16_t error;
        read(fd, &error, sizeof(uint16_t));
        printf("%d", htobe16(error));
        return -1;
    }
    uint32_t length;
    read(fd, &length, sizeof(uint32_t));
    length = htobe32(length);
    char *content = malloc(sizeof(char)*(length+1));
    read(fd, content, sizeof(char)*length);
    content[length] = '\0';
    printf("%s", content);
    free(content);
    return 0;
}

int read_reply_rm(int fd) {
    uint16_t reptype;
    read(fd, &reptype, sizeof(uint16_t));
    reptype = htobe16(reptype);
    if (reptype == SERVER_REPLY_ERROR) {
        char *error = malloc(sizeof(uint16_t));
        if (error == NULL) {
            exit(1);
        }
        read(fd, error, sizeof(uint16_t));
        printf("%s", error);
        free(error);
        return -1;
    }
    return 0;
}

void read_request_cr(int fd){
    struct timing timingTask;
    char *timingString = malloc(sizeof(struct timing));
    struct commandline commandTask;
    read_timing(fd, &timingTask, timingString);
    read_commandline(fd, &commandTask);
    create_tree(&timingTask, &commandTask);
    free_commandline(&commandTask);
    free(timingString);
}

int read_request(int fd){
    uint16_t operation;
    read(fd, &operation, sizeof(uint16_t));
    operation = htobe16(operation);
    switch(operation){
    case CLIENT_REQUEST_LIST_TASKS :
        printf("LS\n");
        break;
    case CLIENT_REQUEST_CREATE_TASK :
        read_request_cr(fd);
        break;
    case CLIENT_REQUEST_REMOVE_TASK :
    
        break;
    case CLIENT_REQUEST_GET_STDOUT :
      
        break;
    case CLIENT_REQUEST_GET_STDERR :
     
        break;
    case CLIENT_REQUEST_TERMINATE :
   
        break;
    case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
       
        break;
  }
  return operation;
}