#include <stdio.h>
#include <string.h>


int main() {
    while (1){
        char entry[300];
        printf("$ ");
        scanf("%[^\n]s",entry);

        if (strcmp(entry,"exit") == 0){
            return 0; //Fechar processo pai
        }

        //Criar um filho com o fork
        // E fazer um exec do que o usuario passou

    }
}
