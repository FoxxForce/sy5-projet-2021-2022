#include "../include/write-pipe.h"

//Envoie une requête CR à saturnd
void write_request_cr(int fd, struct commandline *cl, struct timing *time){
    int position = 0;
    int size = commandline_size(cl) + sizeof(struct timing) + 30;
    char buffer[size];
    memcpy(buffer+position, "CR", sizeof(uint16_t));
    position += sizeof(uint16_t);
    uint64_t minutes = htobe64(time->minutes);
    uint32_t hours = htobe32(time->hours);
    memcpy(buffer+position, &minutes, sizeof(uint64_t));
    position += sizeof(uint64_t);
    memcpy(buffer+position, &hours, sizeof(uint32_t));
    position += sizeof(uint32_t);
    memcpy(buffer+position, &time->daysofweek, sizeof(uint8_t));
    position += sizeof(uint8_t);
    uint32_t  argc = htobe32(cl->ARGC);
    memcpy(buffer+position, &argc, sizeof(uint32_t));
    position += sizeof(uint32_t);
    for(int i=0; i<cl->ARGC; i++){
        uint32_t  length = strlen(cl->ARGV[i]);
        length = htobe32(length);
        memcpy(buffer+position, &length, sizeof(uint32_t));
        position += sizeof(uint32_t);
        memcpy(buffer+position, cl->ARGV[i], htobe32(length));
        position += htobe32(length);
    }
    write(fd, buffer, position);
}
//Envoie la réponse de la requête SO si std=1 sinon celle de SE
int write_reply_so_se(int fd, uint64_t id, int std){
    char file[100];
    if(std==1){
        sprintf(file, "%s/%lu/stdout", TASK_DIR, id);
    }else{
        sprintf(file, "%s/%lu/stderr", TASK_DIR, id);
    }
    
    struct stat st;
    if (stat(file, &st) == -1) {
        return -1;
    } 
    int fd_std = open(file, O_RDONLY);
    if(fd_std==-1){
        return -1;
    }
    char std_string[st.st_size+1];
    read(fd_std, std_string, sizeof(char)*st.st_size);
    std_string[st.st_size] = '\0';
    char buffer[st.st_size+10];
    uint32_t size = st.st_size;
    size = htobe32(size);
    memcpy(buffer, "OK", sizeof(uint16_t));
    memcpy(buffer+sizeof(uint16_t), &size, sizeof(uint32_t));
    memcpy(buffer+sizeof(uint16_t)+sizeof(uint32_t), std_string, st.st_size);
    write(fd, buffer, st.st_size + sizeof(uint16_t)+sizeof(uint32_t));
    close(fd_std);
    return 0;
}

//Envoie la réponse d'une requête TX à cassini
int write_reply_tx(int fd, uint64_t id){
    char file[100];
    struct stat st;
    sprintf(file, "%s/%lu/exitcodes", TASK_DIR, id);
    if (stat(file, &st) == -1) {
            exit(1);
    }
    uint32_t nb_exitcodes = st.st_size/10;
    nb_exitcodes = htobe32(nb_exitcodes);
    int position = 0;
    char buffer[st.st_size + 100];
    memcpy(buffer+position, "OK", sizeof(uint16_t));
    position += sizeof(uint16_t);
    memcpy(buffer+position, &nb_exitcodes, sizeof(uint32_t));
    position += sizeof(uint32_t);
    int64_t time;
    uint16_t exitcode;
    nb_exitcodes = htobe32(nb_exitcodes);
    int fd_exitcodes = open(file, O_RDONLY);
    if(fd_exitcodes==-1){
        return -1;
    }
    for(int i=0; i<nb_exitcodes; i++){
        read(fd_exitcodes, &time, sizeof(int64_t));
        read(fd_exitcodes, &exitcode, sizeof(uint16_t));
        time = htobe64(time);
        exitcode = htobe16(exitcode);
        memcpy(buffer+position, &time, sizeof(int64_t));
        position += sizeof(int64_t);
        memcpy(buffer+position, &exitcode, sizeof(uint16_t));
        position += sizeof(uint16_t);
    }
    write(fd, buffer, position);
    close(fd_exitcodes);
    return 0;
}

//Envoie la réponse d'une requête LS à cassini
void write_reply_ls(int fd){
    struct dirent *dir; 
    DIR *d = opendir(TASK_DIR); 
    uint32_t nbtask = htobe32(nb_task());
    int size = size_reply_ls();
    char buffer[size];
    int position = 0;
    memcpy(buffer+position, "OK", sizeof(uint16_t));
    position += sizeof(uint16_t);
    memcpy(buffer+position, &nbtask, sizeof(uint32_t));
    position += sizeof(uint32_t);
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
            memcpy(buffer+position, &id, sizeof(uint64_t));
            position += sizeof(uint64_t);
            uint64_t minutes = htobe64(time.minutes);
            uint32_t hours = htobe32(time.hours);
            memcpy(buffer+position, &minutes, sizeof(uint64_t));
            position += sizeof(uint64_t);
            memcpy(buffer+position, &hours, sizeof(uint32_t));
            position += sizeof(uint32_t);
            memcpy(buffer+position, &time.daysofweek, sizeof(uint8_t));
            position += sizeof(uint8_t);
            uint32_t  argc = htobe32(cl.ARGC);
            memcpy(buffer+position, &argc, sizeof(uint32_t));
            position += sizeof(uint32_t);
            for(int i=0; i<cl.ARGC; i++){
                uint32_t  length = strlen(cl.ARGV[i]);
                length = htobe32(length);
                memcpy(buffer+position, &length, sizeof(uint32_t));
                position += sizeof(uint32_t);
                memcpy(buffer+position, cl.ARGV[i], htobe32(length));
                position += htobe32(length);
            }
            free_commandline(&cl);
        }
    }
    write(fd, buffer, position);
    closedir(d);
}

int size_reply_ls(){
    char file[100];
    int nb_task = nb_task_created();
    struct stat st;
    int size = nb_task*(sizeof(uint64_t)+ sizeof(uint32_t)+ sizeof(uint8_t));
    for(uint64_t i=1; i<nb_task+1; i++){
        sprintf(file, "%s/%lu/command", TASK_DIR,i);
        if (stat(file, &st) == -1) {
            exit(1);
        }
        size += st.st_size;
    }
    return size + 2000;
}

