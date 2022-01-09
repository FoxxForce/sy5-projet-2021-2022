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

void write_reply_ls(int fd){
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d = opendir(r); 
    write(fd, "OK", sizeof(uint16_t));
    uint32_t nbtask = htobe32(nb_task());
    write(fd, &nbtask, sizeof(uint32_t));
    while ((dir = readdir(d)) != NULL){
        if(strcmp(dir->d_name, ".")!=0 && strcmp(dir->d_name, "..")!=0){
            struct commandline cl;
            struct timing time;
            uint64_t id;
            sscanf(dir->d_name, "%lu", &id);
            if(is_remove_task(id)){
                continue;
            }
            task_commandline(id, &cl);
            task_timing(id, &time);
            id = htobe64(id);
            write(fd, &id, sizeof(uint64_t));
            write_timing_in_pipe(fd, &time);
            write_commandline_in_pipe(fd, &cl);
            free_commandline(&cl);
        }
    }
   
    closedir(d);
}

//Envoie la réponse de la requête SO su std=1 sinon celle de SE
void write_reply_so_se(int fd, uint64_t id, int std){
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d = opendir(r); 
    char file[40];
    if(std==1){
        sprintf(file, "./run/task/%lu/stdout", id);
    }else{
        sprintf(file, "./run/task/%lu/stderr", id);
    }
    int fd_std = open(file, O_RDONLY);
    struct stat st;
    if (stat(file, &st) == -1) {
        printf("t:%d\n", errno);
        exit(1);
    } 
    char std_string[st.st_size+1];
    read(fd_std, std_string, sizeof(char)*st.st_size);
    std_string[st.st_size] = '\0';
    write(fd, "OK", sizeof(uint16_t));
    write(fd, std_string, sizeof(char)*st.st_size);
}