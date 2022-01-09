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
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
void child_handler(int i){
    
}
int main(int argc, char * argv[]) {
    //Créer le répertoire task s'il n'exite pas
    struct stat st = {0};
    if (stat("./run/task", &st) == -1) {
        char * reptask = "./run/task";
        int p = mkdir(reptask , 0744);;
    }

    
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d;
    uint64_t id;
    
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
    time_t timestamp = time(NULL);
    struct tm * tm = localtime(&timestamp);
    while(1){
        timestamp = time(NULL);
        tm = localtime(&timestamp);
        int poll_res = poll(poll_fds, 1, (60-tm->tm_sec)*100);
        if(poll_fds[0].revents == (POLLIN)){
            uint16_t operation = read_request(fd, path_reply_pipe); 
            if(operation==CLIENT_REQUEST_TERMINATE){
                int fd_reply = open(path_reply_pipe, O_WRONLY);
                write(fd_reply, "OK", sizeof(uint16_t));
                close(fd_reply);
                break;
            }
        }
        if(poll_fds[0].revents!=0){
            close(fd);
            fd = open(path_request_pipe, O_RDONLY | O_NONBLOCK);
            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN;
            
        }
        int pid_child;
        pid_t wstatus = waitpid(-1, &wstatus, WNOHANG);
        while(wstatus>0){
            printf("%d\n", pid_child);
            if(WIFEXITED(wstatus)){
                printf("t1t1\n");
                exitcode_task(pid_child, WEXITSTATUS(wstatus));
            }
            exitcode_task(pid_child, 10);
            wstatus = waitpid(-1, &wstatus, WNOHANG);
        }
        d= opendir(r); 
        while ((dir = readdir(d)) != NULL && poll_res==0){
            if(strcmp(dir->d_name, "..")==0 || strcmp(dir->d_name, ".")==0){
                continue;
            }
            sscanf(dir->d_name, "%lu", &id);
            if(!is_remove_task(id)){
                struct timing t;
                struct commandline cl;
                timestamp = time(NULL);
                tm = localtime(&timestamp);
                task_timing(id, &t);
                if(((t.minutes >> (tm->tm_min )) & 1) && ((t.hours >> (tm->tm_hour )) & 1) && 
                    ((t.daysofweek >> (tm->tm_wday )) & 1) && fork() == 0){
                        char id_char[21];
                        sprintf(id_char, "%lu", id);
                        char file[45];
                        sprintf(file, "./run/task/%s/stdout", id_char);
                        printf("%s\n", file);
                        task_commandline(id, &cl);
                        int fd = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd, 1);
                        close(fd);
                        sprintf(file, "./run/task/%s/stderr", id_char);
                        fd = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd, 2);
                        close(fd);
                        sprintf(file, "./run/task/%s/pid", id_char);
                        fd = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        char pid[21];
                        sprintf(pid, "%u", getpid());
                        printf("%s\n", pid);
                        write(fd, pid, sizeof(char)*strlen(pid));
                        close(fd);
                        execvp(cl.ARGV[0], cl.ARGV);
                }
            }
        }
        closedir(d);
    }
    close(fd);
    free(path_reply_pipe);
    free(path_request_pipe);
}











/*while ((dir = readdir(d)) != NULL){
        if(strcmp(dir->d_name, "..")==0 || strcmp(dir->d_name, ".")==0){
            continue;
        }
        sscanf(dir->d_name, "%lu", &id);
        if(!is_remove_task(id)){
            int p = fork ();
            if (p == 0) {
                struct timing t;
                struct commandline cl;
                task_timing(id, &t);
                while(1){
                    time_t timestamp = time(NULL);
                    struct tm * tm = localtime(&timestamp);
                    sleep(60-tm->tm_sec);
                    timestamp = time(NULL);
                    tm = localtime(&timestamp);
                    if(((t.minutes >> (tm->tm_min )) & 1) && ((t.hours >> (tm->tm_hour )) & 1) && 
                        ((t.daysofweek >> (tm->tm_wday )) & 1)){
                            if (fork() == 0) {
                            char id_char[21];
                            sprintf(id_char, "%lu", id);
                            char file[45];
                            sprintf(file, "./run/task/%s/stout", id_char);
                            int fd = open(file,  O_CREAT | O_WRONLY, 0777);
                            //printf("%s\n", file);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                            sprintf(file, "./run/task/%s/sterr", id_char);
                            fd = open(file,  O_CREAT | O_WRONLY, 0777);
                            //printf("%s\n", file);
                            dup2(fd, STDERR_FILENO);
                            close(fd);
                            return exec_task(id);
                            
                    }else{
                        printf("ttuu\n");
                    }
                }
                return -1;
            }
        }
    }
    int retour= -1;
    wait(&retour);
    printf("retour %d\n", WEXITSTATUS(retour));
  char * minutes_str = "14";
  char * hours_str = "17";
  char * daysofweek_str = "1";
  struct timing t;
  timing_from_strings(&t, minutes_str, hours_str, daysofweek_str);
    
    printf("%d", (10 >> (3 - 1)) & 1);
    time_t timestamp = time( NULL );
    struct tm * tm = localtime( & timestamp );
    if(((t.minutes >> (tm->tm_min )) & 1) && ((t.hours >> (tm->tm_hour )) & 1) && 
        ((t.daysofweek >> (tm->tm_wday )) & 1)){
            printf("toto %lu\n", t.minutes);
    }else{
            printf("titi %lu\n", t.minutes);
    }
    
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d = opendir(r); 
    uint64_t id;
    while ((dir = readdir(d)) != NULL){
        if(strcmp(dir->d_name, "..")==0 || strcmp(dir->d_name, ".")==0){
            continue;
        }
        sscanf(dir->d_name, "%lu", &id);
        if(!is_remove_task(id)){
            int p = fork ();
            if (p == 0) {
                struct timing t;
                struct commandline cl;
                task_timing(id, &t);
                while(1){
                    time_t timestamp = time(NULL);
                    struct tm * tm = localtime(&timestamp);
                    sleep(60-tm->tm_sec);
                    timestamp = time(NULL);
                    tm = localtime(&timestamp);
                    if(((t.minutes >> (tm->tm_min )) & 1) && ((t.hours >> (tm->tm_hour )) & 1) && 
                        ((t.daysofweek >> (tm->tm_wday )) & 1)){
                            return exec_task(id);
                            
                    }else{
                        printf("ttuu\n");
                    }
                }
            }
        }
    }
    closedir(d);
    int retour=0;
    wait(&retour);
    printf("retour %d\n", WEXITSTATUS(retour));
    */