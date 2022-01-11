#include "../include/request-saturnd.h" 


uint64_t read_request_cr(int fd){
    struct timing timingTask;
    char *timingString = malloc(sizeof(struct timing));
    struct commandline commandTask;
    read_timing(fd, &timingTask, timingString);
    read_commandline(fd, &commandTask);
    uint64_t id = create_tree(&timingTask, &commandTask);
    free_commandline(&commandTask);
    free(timingString);
    return id;
}

uint16_t read_request(int fd, const char *path_reply_path){
    char buffer[50];
    uint16_t operation;
    read(fd, &operation, sizeof(uint16_t));
    operation = htobe16(operation);
    int fd_reply = -1;
    uint64_t task_id;
    switch(operation){
    case CLIENT_REQUEST_LIST_TASKS :
        fd_reply = open(path_reply_path, O_WRONLY);
        write_reply_ls(fd_reply);
        close(fd_reply);
        break;
    case CLIENT_REQUEST_CREATE_TASK :
        task_id = read_request_cr(fd);
        task_id = htobe64(task_id);
        fd_reply = open(path_reply_path, O_WRONLY);
        memcpy(buffer, "OK", sizeof(uint16_t));
        memcpy(buffer+sizeof(uint16_t), &task_id, sizeof(uint64_t));
        write(fd_reply, buffer, sizeof(uint16_t)+sizeof(uint64_t));
        close(fd_reply);
        break;
    case CLIENT_REQUEST_REMOVE_TASK :
        read(fd, &task_id, sizeof(uint64_t));
        fd_reply = open(path_reply_path, O_WRONLY);
        if(remove_task(htobe64(task_id))==0){
            write(fd_reply, "OK", sizeof(uint16_t));
        }else{
            write(fd_reply, "ERNF", sizeof(uint16_t)*2);
        }  
        close(fd_reply);
        break;
    case CLIENT_REQUEST_GET_STDOUT :
        read(fd, &task_id, sizeof(uint64_t));
        task_id = htobe64(task_id);
        fd_reply = open(path_reply_path, O_WRONLY);
        if(task_id>nb_task_created() || task_id<0){
            write(fd_reply, "ERNF", sizeof(uint16_t)*2);
        }else if(!task_executed(task_id)){
            write(fd_reply, "ERNR", sizeof(uint16_t)*2);
        }else{
            write_reply_so_se(fd_reply, task_id, 1);
        }
        close(fd_reply);

        break;
    case CLIENT_REQUEST_GET_STDERR :
        read(fd, &task_id, sizeof(uint64_t));
        task_id = htobe64(task_id);
        fd_reply = open(path_reply_path, O_WRONLY);
        if(task_id>nb_task_created() || task_id<0){
            write(fd_reply, "ERNF", sizeof(uint16_t)*2);
        }else if(!task_executed(task_id)){
            write(fd_reply, "ERNR", sizeof(uint16_t)*2);
        }else{
            write_reply_so_se(fd_reply, task_id, 2);
        }
        close(fd_reply);
        break;
    case CLIENT_REQUEST_TERMINATE :
        break;
    case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
        read(fd, &task_id, sizeof(uint64_t));
        task_id = htobe64(task_id);
        fd_reply = open(path_reply_path, O_WRONLY);
        if(task_id>nb_task_created() || task_id<0){
            write(fd_reply, "ERNF", sizeof(uint16_t)*2);
        }else{
            write_reply_tx(fd_reply, task_id);
        }
        close(fd_reply);
        break;
  }
  return operation;
}