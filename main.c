#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> //fork()
#include <sys/types.h>
#include <sys/wait.h>

#define TAM 300
#define STG_PIPE "|"


void lower(char *entry)
{
    int i = 0;
    while (i < TAM || entry[i] != '\n' || entry[i] != '\0')
    {
        entry[i] = tolower(entry[i]);
        if (entry[i] == '\n' || entry[i] == '\0')
        {
            return;
        }
        i++;
    }
    entry[i] = '\0';
}

void buscaARG(char *entry, char *args[], int *i)
{
    int qtd = 1;
    char *token = strtok(entry, " ");
    args[*i] = token;
    // printf("ARGS= %s\n", args[i]);

    (*i)++;
    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            break;
        }
        args[*i] = token;
        // printf("args: %s\n",args[i]);
        (*i)++;
    }
    args[*i] = NULL;
}


int montaPipe(char *args[], int tam_arg,char *args_pipe[], int *tam_pipe, int init){

     int i;
     char *args_[15];
     for(i = init;i < tam_arg; i++){
     if(strcmp(args[i], "|") != 0){
             args_pipe[*tam_pipe] = args[i];
             (*tam_pipe)++;
         }else{
//         for(int j = 0 ; j < *tam_pipe; j++){
//             printf("init[%d]ARGUMENTOS[%d]: '%s'\n",init, j, args_pipe[j]);
//         }
             args_pipe[*tam_pipe] = NULL;
             (*tam_pipe)++;
             return (i+1);
             //break;
         }
     }
}


int encontraPipe(char *args[], int tam_arg, int init){
    for(int i = init;i < tam_arg; i++){
    if(strcmp(args[i], STG_PIPE) == 0){
            return 1;
        }
    }
    return 0;
}


int main()
{
    char entry[TAM];
    int pipeFD[2];
    int init = 0;;
    char *command = "ls";

    while (1)
    {
        printf("\n$ ");
        scanf("%[^\n]s", entry);
        setbuf(stdin, NULL);

        lower(entry);

        if (strcmp(entry, "exit") == 0)
        {
            return 0; // Fechar processo pai
        }

        char *args[15];
        char *args_pipe[15];
        int tam_pipe = 0;
        int tam_arg = 0;

        buscaARG(entry,args,&tam_arg);
        int tpipe = encontraPipe(args,tam_arg,0);

        if (tpipe == 1){//Encontrou PIPE

            init = montaPipe(args, tam_arg,args_pipe,&tam_pipe,init);
            int pipefd[2];
            if(pipe(pipefd) == -1) {
                perror("Falha na criacao do pipe");
                exit(1);
            }

            // ========== FILHO A ==========
            if(fork() == 0) //fork do filho A
            {
                dup2(pipefd[1], STDOUT_FILENO);

                close(pipefd[1]);    //libera a ponta de escrita do pipe, já amarrada na saída padrão pelo dup()
                close(pipefd[0]);		//fecha a minha (filho A) ponta de leitura do pipe, pois não utilizarei-

                for(int j = 0 ; j < tam_pipe; j++){
                    printf("ARGUMENTOS1[%d]: '%s'\n", j, args_pipe[j]);
                }

                execvp(args_pipe[0], args_pipe); // substitui o binário do filho A pelo do programa apontado pelo progA
                perror("Falha na substituicao (execvp) do filho A pelo programa ls");
                exit(1); //se o exec der errado, fecha o processo filho A pois não faz sentido continuar
            } // filho A

            // ========== FILHO B ==========
            pid_t pidB = fork();
            if(pidB == 0) //fork do filho B
            {
                dup2(pipefd[0], STDIN_FILENO); //substitui a entrada padrão pela minha (filho B) ponta de leitura do pipe

                // ## Como usar o dup() ao invés do dup2():
                // close(STDIN_FILENO);   	//fecha a entrada padrão (teclado), que não mais utilizarei no filho B
                // dup(des_p[READ_END]);   //substitui a entrada padrão pela minha (filho B) ponta de leitura do pipe

                close(pipefd[0]);
                close(pipefd[1]);
                //OBS.: no filh B será mantida aberta a saída padrão (monitor), para exibir dados ao usuário


                tam_pipe = 0;
                montaPipe(args, tam_arg,args_pipe,&tam_pipe,init);
                args_pipe[tam_pipe] = NULL;
                tam_pipe++;
                for(int j = 0 ; j < tam_pipe; j++){
                    printf("ARGUMENTOS2[%d]: '%s'\n", j, args_pipe[j]);
                }

                execvp(args_pipe[0], args_pipe);
                perror("Falha na substituicao (execvp) do filho B pelo programa tee");
                exit(1); //se o exec der errado, fecha o processo filho B pois não faz sentido continuar
            }

            close(pipefd[1]); 
            close(pipefd[0]);//0 leitura
            waitpid(pidB, NULL, 0);
        }else{
            //Criar um filho com o fork
            pid_t pid = fork();
            if (pid == -1) {      /* fork() failed */
                perror("Erro no fork");
                exit(EXIT_FAILURE);
            }

            //char* argument_list[] = {"ls", "-l", NULL};
            if (pid == 0){
                //printf("Processo Filho entry: %s",entry);
                execvp(args[0], args);
                //execvp("ls", argument_list); 
                perror("execvp");
                exit(0);
            }

            wait(&pid); //Aguardar o processo filho terminar 
            //printf("\nPID Filho: %d",pid);
            // E fazer um exec do que o usuario passou
        }
    }
}
