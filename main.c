#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> //fork()
#include <sys/types.h>
#include <sys/wait.h>

#define TAM 300
#define STG_PIPE "|"
#define STG_INPUT_FILE "<"  
//cat -n < arquivo.txto comando irá exibir as linhas do arquivo arquivo.txt numeradas.
#define STG_OUTPUT_FILE ">" 
//O comando echo "ola mundo" > arquivo.txt escreve a string "ola mundo" no arquivo de nome "arquivo.txt". Se o arquivo não existir, ele será criado. Se já existir, seu conteúdo será substituído pelo novo conteúdo.
#define PONTA_LEITURA 0 
#define PONTA_ESCRITA 1


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


int argsPipe(char *args[], int tam_arg,char *args_pipe[], int *tam_pipe, int init){
    int i;
    for(i = init;i < tam_arg; i++){
    if(strcmp(args[i], "|") != 0){
            args_pipe[*tam_pipe] = args[i];
            (*tam_pipe)++;
        }else{
            args_pipe[*tam_pipe] = NULL;
            (*tam_pipe)++;
            return (i+1);
        }
    }
    return -1;
}


int verificaPipe(char *args[], int tam_arg, int init){
    int qtd = 0;
    for(int i = init;i < tam_arg; i++){
        if(strcmp(args[i], STG_PIPE) == 0){
                qtd++;
            }
    }
    return qtd;
}

void criaPipe(int *pipefd){
    if(pipe(pipefd) == -1) { // Criar um pipe
        perror("Falha na criacao do pipe");
        exit(1);
    }
}

void readFile(FILE *fp,char namefile[]){
    if ((fp = fopen(namefile, "r")) == NULL){
        perror("Falha na Leitura");
        exit(1);
    }else{
        char entry[120];
        while (!feof(fp)){
            fgets(entry,120, fp);
            printf("\n%s",entry);
        }
    }
}

void writeFile(FILE *fp,char namefile[],char text[120]){
    if ((fp = fopen(namefile, "W")) == NULL){
        printf("Erro ao criar arquivo");
    }else{
        fprintf(fp, "%s",text);
    }
}


pid_t do_fork(int pipefd[],int pontaPipe,int old_decriptor,char *args[]){//PIPE Direita descriptor = 1, old_decriptor = STDOUT_FILENO                                                     
    pid_t pidB = fork();
    if(pidB == 0){ //BLOCO de exec para o fork
        dup2(pipefd[pontaPipe], old_decriptor);

        close(pipefd[pontaPipe]);    //libera as pontas de escrita e leitura do pipe
        if (pontaPipe == 1){
            close(pipefd[0]);		
        }else{
            close(pipefd[1]);
        }
        execvp(args[0], args); // substitui o binário do filho A pelo do programa apontado pelo progA
        perror("Falha na execvp");
        exit(1); //se o exec der errado, fecha o processo filho A pois não faz sentido continuar
    } // filho A
    return pidB; 
}


void IOfile(char *args[], int tam_arg, int init){
    char namefile[120];
    char *args_[15];
    int tamArgs = 0;
    for(int i = init;i < tam_arg; i++){
        if(strcmp(args[i], STG_OUTPUT_FILE) == 0){ //> Salva no arquivo 
            strcpy(namefile,args[i+1]);//NameFILE

            FILE *fp = fopen(namefile,"r");
            int fp = fileno(fp); //descritor do arquivo 

            

        }else if (strcmp(args[i], STG_INPUT_FILE) == 0){ //< Alterar saida de dados para a escrita no arquivo
            strcpy(namefile,args[i+1]);//NameFILE
            printf("{%s,%s,%s}",args_[0],args_[1],args_[2]);
        }else{
            args_[tamArgs++] = args[i];
        }
    }
}


int main()
{
    char entry[TAM];
    
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
        int tam_arg = 0;
        buscaARG(entry,args,&tam_arg);

        char *args_pipe[15];
        int tam_pipe = 0;
        int tpipe = verificaPipe(args,tam_arg,0); //quantidade de pipes existente

        int init = 0; // Variavel de controle sobre a busca de argumentos para pipe(Guarda o INDEX do ultimo pipe + 1)
        if (tpipe >= 1){//TEM PIPE

            init = argsPipe(args, tam_arg,args_pipe,&tam_pipe,init); //Encontrar Primeiro Argumento

            int pipefd[2];
            criaPipe(pipefd);
            // ========== FILHO A ==========
            // Alterar o descriptor de SAIDA padrão (Monitor) para a ponta de LEITURA do PIPE - processo a direita 
            do_fork(pipefd,PONTA_ESCRITA,STDOUT_FILENO,args_pipe);
            
            
            tam_pipe = 0;
            init = argsPipe(args, tam_arg,args_pipe,&tam_pipe,init); //Busca de Argumentos 

            // ========== FILHO B ==========
            // Alterar o descriptor de ENTRADA padrão (Teclado) para a ponta de LEITURA do PIPE - processo a direita 
            pid_t pidb = do_fork(pipefd,PONTA_LEITURA,STDIN_FILENO,args_pipe);


            //Encerrar DUP2
            close(pipefd[1]);
            close(pipefd[0]);//0 leitura
            
            waitpid(pidb,NULL,0); // Aguardar o ultimo processo ser finalizado
        }else{

            IOfile(args,tam_arg,0);
            //Criar um filho com o fork 
            pid_t pid = fork();
            if (pid == -1) {      /* fork() failed */
                perror("Erro no fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0){
                execvp(args[0], args); //executar comando basico 
                perror("execvp");
                exit(0);
            }

            waitpid(pid,NULL,0);//Aguardar o processo filho terminar 
        }
    }
}

