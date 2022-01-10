#include "../include/saturnd.h" 

int selfpipe[2];
void child_handler(){
    write(selfpipe[1], "",1);
}

int main(int argc, char * argv[]) {
    //Créer le répertoire task s'il n'exite pas
    struct stat st = {0};
    if (stat(SATURND_DIR, &st) == -1) {
        mkdir(SATURND_DIR , 0744);
        mkdir(PIPES_DIR , 0744);
        mkdir(TASK_DIR , 0744);
        mkfifo(PIPE_REQUEST , 0744);
        mkfifo(PIPE_REPLY , 0744);
    }

    
    struct dirent *dir; 
    DIR *d;
    uint64_t id;
    uint16_t operation;
    int fd = open(PIPE_REQUEST , O_RDONLY | O_NONBLOCK);
    
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
        printf("toto\n");
        //int poll_res = poll(poll_fds, 2, 5000);
        if(poll_fds[1].revents == (POLLIN)){
            while (read(selfpipe[0], buffer, sizeof(buffer)) > 0){}
        }
        if(poll_fds[0].revents == (POLLIN)){
            uint16_t operation = read_request(fd, PIPE_REPLY); 
            if(operation==CLIENT_REQUEST_TERMINATE){
                int fd_reply = open(PIPE_REPLY, O_WRONLY);
                write(fd_reply, "OK", sizeof(uint16_t));
                close(fd_reply);
                kill_childs();
                break;
            }
        }
        if(poll_fds[0].revents!=0){
            close(fd);
            fd = open(PIPE_REQUEST, O_RDONLY | O_NONBLOCK);
            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN;
            
        }
        pid_child = waitpid(-1, &wstatus, WNOHANG);
        while(pid_child>0){
            if(WIFEXITED(wstatus)){
                exitcode_task(pid_child, WEXITSTATUS(wstatus));
            }else{
                exitcode_task(pid_child, 0xFFFF);
            }
            pid_child = waitpid(-1, &wstatus, WNOHANG);
        }
        d= opendir(TASK_DIR); 
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
                        char file[150];
                        sprintf(file, "%s/%s/stdout", TASK_DIR, id_char);
                        printf("%s\n", file);
                        task_commandline(id, &cl);
                        int fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd2, 1);
                        close(fd2);
                        sprintf(file, "%s/%s/stderr",TASK_DIR, id_char);
                        fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        dup2(fd2, 2);
                        close(fd2);
                        sprintf(file, ".%s/%s/pid", TASK_DIR, id_char);
                        fd2 = open(file,  O_CREAT | O_WRONLY | O_TRUNC, 0777);
                        char pid[21];
                        sprintf(pid, "%u", getpid());
                        write(fd, pid, sizeof(char)*strlen(pid));
                        close(fd2);
                        sprintf(file, "%s/%s/exitcodes", TASK_DIR, id_char);
                        fd2 = open(file, O_WRONLY | O_APPEND);
                        uint64_t secondes = time(NULL);
                        write(fd2, &secondes, sizeof(uint64_t));
                        close(fd2);
                        close(selfpipe[0]);
                        close(selfpipe[1]);
                        close(fd);
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
}