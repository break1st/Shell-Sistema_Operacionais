#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> //fork()
#include <sys/types.h>
#include <sys/wait.h>

#define TamEntry 300


void lower(char *entry){
    int i = 0;
    while (i < TamEntry || entry[i] != '\n' || entry[i] != '\0')
    {
        entry[i] = tolower(entry[i]);
        if( entry[i] == '\n' || entry[i] == '\0'){return;}
        i++;
    }
    entry[i] = '\0';
}

int main() {
    char entry[TamEntry];
    pid_t pid;
    

    char* command = "ls";

    while (1){
        printf("\n$ ");
        scanf("%[^\n]s",entry);
        setbuf(stdin,NULL);

        lower(entry);
        
        if (strcmp(entry,"exit") == 0){
            return 0; //Fechar processo pai
        }
        //fatiar string 
        int i = 0;
        int j=0;

        char* args[10];
        int qtd = 1;
        char *token = strtok(entry, " ");
        args[i] = token;
        //printf("ARGS= %s\n", args[i]);

        i++;
        while(token != NULL){
            token = strtok(NULL, " ");
            if (token == NULL){
                break;
            }
            args[i] = token;
            //printf("args: %s\n",args[i]);
            i++;
        }
        args[i] = NULL;
        i++;
        
        // for(int j=0; j < i; j++){
        //     printf("ARGS[%d] = %s\n", j, args[j]);
        // }

        //Criar um filho com o fork
        pid = fork();
        if (pid == -1) {      /* fork() failed */
            perror("Erro no fork");
            exit(EXIT_FAILURE);
        }
        
        //char* argument_list[] = {"ls", "-l", NULL};
        if (pid == 0){
            printf("Processo Filho entry: %s",entry);
            execvp(entry, args); 
            //execvp("ls", argument_list); 
            perror("execvp");
            exit(0);
        }   

        wait(&pid); //Aguardar o processo filho terminar 
        printf("\nPID Filho: %d",pid);
        // E fazer um exec do que o usuario passou


    }
}
