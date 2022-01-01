#include "../include/write-pipe.h"

void write_timing_in_pipe(int fd, struct timing *time){
    uint64_t minutes = htobe64(time->minutes);
    uint32_t hours = htobe32(time->hours);
    write(fd, &minutes, sizeof(uint64_t));
    write(fd, &hours, sizeof(uint32_t));
    write(fd, &time->daysofweek, sizeof(uint8_t));
}

void write_commandline_in_pipe(int fd, struct commandline *cl){
    uint32_t  argc = htobe32(cl->ARGC);
    write(fd, &argc, sizeof(uint32_t));
    for(int i=0; i<cl->ARGC; i++){
        uint32_t  length = strlen(cl->ARGV[i]);
        length = htobe32(length);
        write(fd, &length, sizeof(uint32_t));
        write(fd, cl->ARGV[i], htobe32(length));
    }
}