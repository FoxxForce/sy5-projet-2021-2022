#include "../include/read-pipe.h"
void read_reply_cr(int fd){
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
    }
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
    printf("%s ", timingString);
}

void read_commandline(int fd, struct commandline *commandTask){
    read(fd, &commandTask->ARGC, sizeof(uint32_t));
    commandTask->ARGC = htobe32(commandTask->ARGC);
    char *argv[commandTask->ARGC];
    commandTask->ARGV = argv;
    uint32_t length;
    for(int i=0; i<commandTask->ARGC; i++){
        read(fd, &length, sizeof(uint32_t));
        length = htobe32(length);
        commandTask->ARGV[i] = malloc(sizeof(char)*length);
        read(fd, commandTask->ARGV[i], length);
    }
    print_commandline(commandTask);
}

void read_id(int fd){
    uint64_t id;
    read(fd, &id, sizeof(uint64_t));
    id = htobe64(id);
    printf("%ld: ", id);
}