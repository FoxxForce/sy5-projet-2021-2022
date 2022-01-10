#include "../include/saturnd.h" 

int selfpipe[2];
void child_handler(){
    write(selfpipe[1], "",1);
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
    
    pipe(selfpipe);
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = child_handler;
    sigaction(SIGCHLD, &act, NULL);
    struct pollfd poll_fds[2];
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;
    poll_fds[1].fd = selfpipe[1];
    poll_fds[1].events = POLLIN;
    time_t timestamp;
    struct tm * tm;
    char buffer[1024];
    int wstatus;
    int pid_child;
    while(1){
        timestamp = time(NULL);
        tm = localtime(&timestamp);
        int poll_res = poll(poll_fds, 2, (60-tm->tm_sec)*1000);
        //int poll_res = poll(poll_fds, 2, 5000);
        if(poll_fds[1].revents == (POLLIN)){
            while (read(selfpipe[0], buffer, sizeof(buffer)) > 0){}
        }
        if(poll_fds[0].revents == (POLLIN)){
            uint16_t operation = read_request(fd, path_reply_pipe); 
            if(operation==CLIENT_REQUEST_TERMINATE){
                int fd_reply = open(path_reply_pipe, O_WRONLY);
                write(fd_reply, "OK", sizeof(uint16_t));
                close(fd_reply);
                kill_childs();
                break;
            }
        }
        if(poll_fds[0].revents!=0){
            close(fd);
            fd = open(path_request_pipe, O_RDONLY | O_NONBLOCK);
            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN;
            
        }
        pid_child = waitpid(-1, &wstatus, WNOHANG);
        while(pid_child>0){
            printf("ssokss\n");
            if(WIFEXITED(wstatus)){
                exitcode_task(pid_child, WEXITSTATUS(wstatus));
            }else{
                exitcode_task(pid_child, 0xFFFF);
            }
            pid_child = waitpid(-1, &wstatus, WNOHANG);
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
                        int fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd2, 1);
                        close(fd2);
                        sprintf(file, "./run/task/%s/stderr", id_char);
                        fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd2, 2);
                        close(fd2);
                        sprintf(file, "./run/task/%s/pid", id_char);
                        fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        char pid[21];
                        sprintf(pid, "%u", getpid());
                        write(fd, pid, sizeof(char)*strlen(pid));
                        close(fd2);
                        sprintf(file, "./run/task/%s/exitcodes", id_char);
                        fd2 = open(file, O_WRONLY | O_APPEND);
                        uint64_t secondes = time(NULL);
                        write(fd2, &secondes, sizeof(uint64_t));
                        close(fd2);
                        close(selfpipe[0]);
                        close(selfpipe[1]);
                        close(fd);
                        free(path_reply_pipe);
                        free(path_request_pipe);
                        closedir(d);
                        int r = execvp(cl.ARGV[0], cl.ARGV);
                        free_commandline(&cl);
                        return r;
                        
                }
            }
        }
        closedir(d);
    }
    pid_child = waitpid(-1, &wstatus, WNOHANG);
    while(pid_child>0){
            printf("ok\n");
            if(WIFEXITED(wstatus)){
                exitcode_task(pid_child, WEXITSTATUS(wstatus));
            }else{
                exitcode_task(pid_child, 0xFFFF);
            }
            pid_child = waitpid(-1, &wstatus, WNOHANG);
    }
    close(selfpipe[0]);
    close(selfpipe[1]);
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