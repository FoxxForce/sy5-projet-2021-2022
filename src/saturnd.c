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
#include <poll.h>
#include <sys/types.h>
#include <dirent.h>
int main(int argc, char * argv[]) {
    //Créer le répertoire task s'il n'exite pas
    struct stat st = {0};
    if (stat("./run/task", &st) == -1) {
        char * reptask = "./run/task";
        int p = mkdir(reptask , 0744);;
    }
    

    uint16_t operation;
    char * pipes_directory = "./run/pipes";
    if(pipes_directory==NULL){
      char *username = getenv("USER");
      pipes_directory = malloc(500);
      sprintf(pipes_directory, "/tmp/%s/saturnd/pipes", username);
    }
    
    
    char * path_request_pipe = malloc(sizeof(char)*1000);
    sprintf(path_request_pipe, "%s/saturnd-request-pipe", pipes_directory);
    int fd = open(path_request_pipe , O_RDONLY | O_NONBLOCK);

    char * path_reply_pipe = malloc(sizeof(char)*1000);
    sprintf(path_reply_pipe, "%s/saturnd-reply-pipe", pipes_directory);

    struct pollfd poll_fds[1];
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;
    while(1){
        int poll_res = poll(poll_fds, 1, -1);
        if(poll_fds[0].revents == (POLLIN)){
            uint16_t operation = read_request(fd, path_reply_pipe); 
        }
        if(poll_fds[0].revents!=0){
            close(fd);
            fd = open(path_request_pipe, O_RDONLY | O_NONBLOCK);
            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN;
            
        }
        
    }
    close(fd);
    free(path_reply_pipe);
    free(path_request_pipe);


}