#include "../include/commandline.h"

/* La fonction commandline_from_arguments récupére la commande à exécute dans argv et l'écrit dans
 dest. Renvoie 0 en cas de succès sinon -1*/
int commandline_from_arguments(struct commandline *dest, int argc, char **argv) {
    int c = -1;
    for(int i=0; i<argc; i++){
        if(strcmp("-c", argv[i])==0){
            c = i+1;
            break;
        }
    }
    if(c==-1){
        return -1;
    }
  
    while(strcmp("-d", argv[c])==0 || strcmp("-H", argv[c])==0 || strcmp("-m", argv[c])==0){
        c = c+2;
    }
    
    dest->ARGV = malloc(sizeof(char*)*(argc-c));
    memset(dest->ARGV, 0, sizeof(char*)*(argc-c));
    int length = 0;
    for (int i=0; i<argc-c; i++) {
        length = strlen(argv[c+i]);
        dest->ARGV[i] = malloc(sizeof(char)*(length+1));
        memset(dest->ARGV[i], 0, length+1);
        if(dest->ARGV[i]==NULL){
            return -1;
        }
        strncpy(dest->ARGV[i], argv[c+i], length);
        dest->ARGV[i][length] = '\0';
    }
    dest->ARGC = argc - c;
    return 0;
}

void free_commandline(struct commandline *cl){
    for(int i=0; i<cl->ARGC; i++) {
        free(cl->ARGV[i]);
    }
    free(cl->ARGV);
}

void print_commandline(struct commandline *cl){
    for(int i=0; i<cl->ARGC; i++){
       printf("%s ", cl->ARGV[i]);
    }
    printf("\n");
}

int commandline_size(const struct commandline *cl){
    int size = 0;
    for(int i=0; i<cl->ARGC; i++){
        size = size + strlen(cl->ARGV[i]) + 1;
    }
    return size+1;
}
void commandline_string_from_commandline(char * dest, const struct commandline *cl){
    int size = 0;
    strcpy(dest, cl->ARGV[0]);
    for(int i=1; i<cl->ARGC; i++){
        strcat(dest, "\n");
        strcat(dest, cl->ARGV[i]);
        
    }
}

void exec_commandline(struct commandline *cl){
    execvp(cl->ARGV[0], cl->ARGV);
}
