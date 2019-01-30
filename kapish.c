#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
int setenv(const char *var_name, const char *new_value,int change_flag);
int unsetenv(const char *var_name);
//int chdir();


#define INPUT_BUFFER 512
#define TOK_BUFFER 69
#define TOK_DELIM " \t\r\n\a"



 char *built_in[] = {"cd","setenv","unsetenv","exit"};


int shell_launcher(char **args){
    
    pid_t p_id;
    //int status;

    p_id = fork();

    if(p_id == 0){
        execvp(args[0],args);
        fprintf(stderr,"kapish: Invalid Command \n");
        exit(EXIT_FAILURE);
    } else if (p_id < 0) {
        fprintf(stderr,"kapish: fork error \n");
    } else {
       //do{
            wait(NULL);
        //}while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int shell_cd(char **args){
    if(args[1]!=NULL){
        if(chdir(args[1])){
            fprintf(stderr,"kapish: invalid path for cd \n");
        }
    } else {fprintf(stderr,"kapish: need argument for cd \n");}

    return 1;
}

int shell_setenv(char **args){ 
    if(args[1]!=NULL){

        if( setenv(args[1], args[2], 1) ){
            fprintf(stderr,"kapish: invalid argument for setenv \n");
        }

        getenv(args[1]);

    } else{fprintf(stderr,"kapish: argument needed for setenv \n");}

    return 1;
}

int shell_unsetenv(char **args){
    if(args[1]!=NULL){
          
          if(unsetenv(args[1]) != 0){
            fprintf(stderr,"kapish: invalid argument for unsetenv \n");
        }
    } else {fprintf(stderr,"kapish: argument needed for unsetenv \n");}

    return 1;
}

int shell_exit(){
    return 0;
}

char *read_line(void){

    int buffer_size = INPUT_BUFFER;
    int pos = 0;
    char *buf = malloc(sizeof(char) * buffer_size);
    int c;

    if(!buf){
        fprintf(stderr, "kapish: allocation error\n");
        exit(EXIT_FAILURE);
    }


    while(1){
        c=getchar();

        if(c == EOF || c == '\n' ){
            buf[pos] = '\0';
            return buf;
        } else {
            buf[pos]=c;
        }

        pos++;

        if(pos>=buffer_size){
            buffer_size += INPUT_BUFFER;
            buf = realloc(buf,buffer_size*sizeof(char));

            if(!buf){
            fprintf(stderr, "kapish: allocation error\n");
            exit(EXIT_FAILURE);
            }
        }
    }

}

char **tokenize(char *input){
    int buffer_size = TOK_BUFFER;
    int pos = 0;
    char **toks = malloc(sizeof(char) * buffer_size);
    char *tok;

    if(!toks){
        fprintf(stderr, "kapish: allocation error\n");
        exit(EXIT_FAILURE);
    }

    tok = strtok(input, TOK_DELIM);
    
    while(tok !=NULL){
        toks[pos] = tok;
        pos++;

        if(pos>=buffer_size){
            buffer_size += TOK_BUFFER;
            toks = realloc(toks,buffer_size * sizeof(char));

            if(!toks){
            fprintf(stderr, "kapish: allocation error\n");
            exit(EXIT_FAILURE);
            }

        }
       
       tok = strtok(NULL,TOK_DELIM);
  
    }

    toks[pos] = NULL;
    return toks;

}

int execute (char **args){
    
    if(args[0]==NULL){return 1;}
    
    int index =0;
    int size = sizeof(built_in) / sizeof(char*);

    for(index=0; index<size;index++){
        if(strcmp(built_in[index],args[0])==0){break;}
    }

    switch(index){
        case 0:{
            return shell_cd(args);
        }
        case 1:{
            return shell_setenv(args);
        }
        case 2:{
            return shell_unsetenv(args);
        }
        case 3:{
            return shell_exit();
        }
        default:{
            return shell_launcher(args);
        }
    }

    
    return 0;
}

void shell_loop(void){

    char *line;
    char **args;
    int status;

    do{

    printf("? ");
    line = read_line();
    args = tokenize(line);
    status = execute(args);

    free(line);
    free(args);

    }while(status);

}


int main(){

    shell_loop();

    return EXIT_SUCCESS;
}
