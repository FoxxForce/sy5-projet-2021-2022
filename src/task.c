#include "../include/task.h"
uint64_t create_tree(struct timing *time, struct commandline *cl){
    //Récupérer le nombre de tâches
    uint64_t nbtask = nb_task_created();
    
    
    char timing_string[TIMING_TEXT_MIN_BUFFERSIZE];
    timing_string_from_timing(timing_string, time);
    char id[21];
    sprintf(id, "%lu", nbtask+1);
    char repTask[40];
    sprintf(repTask, "./run/task/%s", id);
    mkdir(repTask, 0700);
    char file[45];
    sprintf(file, "./run/task/%s/time", id);
    int fd = open(file,  O_CREAT | O_WRONLY, 0777);
    write(fd, timing_string, sizeof(char)*strlen(timing_string));
    close(fd);

    sprintf(file, "./run/task/%s/command", id);
    fd = open(file,  O_CREAT | O_WRONLY, S_IRWXU);
    int cl_size = commandline_size(cl);
    char commandline_string[cl_size];
    commandline_string_from_commandline(commandline_string, cl);
    write(fd, commandline_string, sizeof(char)*strlen(commandline_string));
    close(fd);

    sprintf(file, "./run/task/%s/id", id);
    fd = open(file,  O_CREAT | O_WRONLY, S_IRWXU);
    write(fd, id, sizeof(char)*strlen(id));
    close(fd);
    
    sprintf(file, "./run/task/%s/exitcodes", id);
    fd = open(file,  O_CREAT);
    close(fd);
    
    return nbtask+1;
}

int nb_task_created(){
    int nbtask = 0;
    struct dirent *dir; 
    char *r = "./run/task";
    DIR *d = opendir(r); 
    while ((dir = readdir(d)) != NULL){
        nbtask++;
    }
    nbtask -= 2;
    closedir(d);
    return nbtask;
}
int nb_task(){
    int nbtask = 0;
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
            nbtask++;
        }
    }
    closedir(d);
    return nbtask;
}

int is_remove_task(int task_id){
    char task[40];
    sprintf(task, "./run/task/%d", task_id);
    struct dirent *dir; 
    DIR *d = opendir(task); 
    char file[1024];
    while ((dir = readdir(d)) != NULL){
        if(strcmp(dir->d_name, "remove")==0 ){
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}

//Ajoute un fichier nommé `remove` dans le répertoire de la tâche task_id pour supprimer la tâche
int remove_task(int task_id){
    char task[40];
    sprintf(task, "./run/task/%d", task_id);
    struct stat st = {0};
    if (stat(task, &st) == -1 || is_remove_task(task_id)) {
        return -1;
    }
    char remove[50];
    sprintf(remove, "%s/remove", task);
    int fd = open(remove,  O_CREAT , 0777);
    close(fd);
    return 0;
}

void task_commandline(uint64_t id, struct commandline *cl){
    char file[40];
    sprintf(file, "./run/task/%lu/command", id);
    struct stat st;
    if (stat(file, &st) == -1) {
        printf("c:%d\n", errno);
        exit(1);
    }
    char command[st.st_size + 2];
    int fd = open(file, O_RDONLY); 
    read(fd, command, st.st_size);      
    command[st.st_size] = '\n';
    command[st.st_size+1] = '\0';
    int nb_words = 1;
    for(int i=0; i<st.st_size; i++){
        if(command[i]=='\n'){
            nb_words++;
        }
    }
    char **argv = malloc(sizeof(char *)*(1+nb_words));
    cl->ARGC = nb_words;
    cl->ARGV = argv;
    nb_words = 0;
    int nb_char = 0;
    for(int i=0; i<st.st_size+1; i++){
        if(command[i]=='\n'){
            char *word = malloc(sizeof(char)*(nb_char+1));
            for(int j=0; j<nb_char; j++){
                word[j] = command[i-nb_char+j];
            }
            word[nb_char] = '\0';
            cl->ARGV[nb_words] = word;
            nb_words++;
            nb_char = 0;
            continue;
        }
        nb_char++;
    }
    argv[nb_words] = NULL;
}

void task_timing(uint64_t id, struct timing *time){
    char file[40];
    sprintf(file, "./run/task/%lu/time", id);
    struct stat st;
    if (stat(file, &st) == -1) {
        printf("t:%d\n", errno);
        exit(1);
    }
    char timing_string[st.st_size+2];
    int fd = open(file, O_RDONLY); 
    read(fd, timing_string, st.st_size); 
    timing_string[st.st_size] = ' ';     
    timing_string[st.st_size+1] = '\0';
    char **argv = malloc(sizeof(char *)*3);
    int nb_char = 0;
    int nb_words = 0;
    for(int i=0; i<st.st_size+1; i++){
        if(timing_string[i]==' '){
            char *word = malloc(sizeof(char)*(nb_char+1));
            for(int j=0; j<nb_char; j++){
                word[j] = timing_string[i-nb_char+j];
            }
            word[nb_char] = '\0';
            argv[nb_words] = word;
            nb_words++;
            nb_char = 0;
            continue;
        }
        nb_char++;
    }
     timing_from_strings(time, argv[0], argv[1], argv[2]);
     for(int i=0; i<nb_words; i++){
         free(argv[i]);
     }
     free(argv);
}

//renvoie 1 si la tâche a été exécutée au moins une fois sinon 0
int task_executed(uint64_t id){
    char file[40];
    sprintf(file, "./run/task/%lu/stdout", id);
    struct stat st;
    if (stat(file, &st) == -1) {
        return 0;
    }
    return 1;
}