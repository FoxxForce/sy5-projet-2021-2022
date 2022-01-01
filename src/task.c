#include "../include/cassini.h"
#include <sys/dir.h>
int create_tree(struct timing *time, struct commandline *cl){
    //Récupérer le nombre de tâches
    int nbtask = -2;
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d = opendir(r); 
    
    while ((dir = readdir(d)) != NULL){
        nbtask++;
    }
    closedir(d);
    char timing_string[TIMING_TEXT_MIN_BUFFERSIZE];
    timing_string_from_timing(timing_string, time);
    char id[12];
    sprintf(id, "%d", nbtask+1);
    char repTask[30];
    sprintf(repTask, "./run/task/%s", id);
    mkdir(repTask, 0700);
    char file[45];

    sprintf(file, "./run/task/%s/time", id);
    printf("%s\n%s\n", id, file);
    int fd = open(file,  O_CREAT | O_WRONLY, 0777);
    write(fd, timing_string, sizeof(char)*strlen(timing_string));
    printf("%d ss\n", errno);
    close(fd);
    print_commandline(cl);
    sprintf(file, "./run/task/%s/command", id);
    fd = open(file,  O_CREAT | O_WRONLY, S_IRWXU);
    int cl_size = commandline_size(cl);
    char commandline_string[cl_size];
    commandline_string_from_commandline(commandline_string, cl);
    printf("%s\n", commandline_string);
    write(fd, commandline_string, sizeof(char)*strlen(commandline_string));
    close(fd);

    return 1;
}