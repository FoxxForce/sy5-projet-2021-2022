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
        int p = mkdir(reptask , 0744);
       // printf("oui %d\n", p);
    }
    

    uint16_t operation;
    char * pipes_directory = "./run/pipes";
    if(pipes_directory==NULL){
      char *username = getenv("USER");
      pipes_directory = malloc(500);
      sprintf(pipes_directory, "/tmp/%s/saturnd/pipes", username);
    }
    int attributs;
    char * request_pipe = "/saturnd-request-pipe";
    char * chemin = malloc(sizeof(char)*1000);
    strncat(chemin, pipes_directory, strlen(pipes_directory));
    strncat(chemin, request_pipe, strlen(request_pipe));
    int fd = open(chemin, O_RDONLY | O_NONBLOCK);
    struct pollfd poll_fds[1];
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;
    while(1){
        int poll_res = poll(poll_fds, 1, -1);
        printf("%d\n", poll_fds[0].revents);
        if(poll_fds[0].revents == (POLLIN)){
            sleep(1);
            uint16_t operation = read_request(fd);   
        }
        if(poll_fds[0].revents!=0){
            close(fd);
            fd = open(chemin, O_RDONLY | O_NONBLOCK);
            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN;
            
        }
        
    }
    /*char * request_pipe2 = "/saturnd-reply-pipe";
    char * chemin2 = malloc(sizeof(char)*1000);
    strncat(chemin2, pipes_directory, strlen(pipes_directory));
    strncat(chemin2, request_pipe2, strlen(request_pipe));
    int fd2 = open(chemin2, O_RDONLY );*/


}