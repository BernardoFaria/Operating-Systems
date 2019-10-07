#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>           // Clock
#include <pthread.h>        // Threads
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

void *applyCommands();
pthread_mutex_t mutex;

int numberThreads = 0;
tecnicofs* fs;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName){
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}


/* Recebe um ficheiro input */
void processInput(FILE *inputFile){
    char line[MAX_INPUT_SIZE];

    /* recebe o inputFile em vez do stdin */
    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            case '#':
                break;
            default: { /* error */
                errorParse();
            }
        }
    }
}

void *applyCommands(){

    pthread_mutex_lock(&mutex);

    while(numberCommands > 0){
        const char* command = removeCommand();
        if (command == NULL){
            continue;
        }

        char token;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s", &token, name);
        if (numTokens != 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        int iNumber;
        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);
                create(fs, name, iNumber);
                break;
            case 'l':
                searchResult = lookup(fs, name);
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                delete(fs, name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    fs = new_tecnicofs();


    /* Cria file, abre input file para ler */
    FILE *inputFile;
    char *inputName = argv[1];
    inputFile = fopen(inputName, "r");


    processInput(inputFile);


    /* Fecha input file */
    fclose(inputFile);

    /* Criação das tarefas */
    numberThreads = atoi(argv[3]); // atoi converte string para int
    pthread_t tid[numberThreads];
    
    for(int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tid[i], 0, applyCommands, inputCommands[i]) == 0) {
            printf("Criada a tarefa %ld\n", tid[i]);
        }
        else {
            printf("Erro na criação da tarefa\n");
            exit(1);
        }
    }

    for(int i = 0; i < numberThreads; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("Terminaram todas as threads\n");

    /* Inicio do relogio */
    clock_t start_t = clock();

    /* Cria file, abre ou cria output file para escrever */
    FILE *outputFile;
    char *outputName = argv[2];
    outputFile = fopen(outputName, "a");
    
    
    /* Recebe o output File para escrever */
    print_tecnicofs_tree(outputFile, fs);

    /* Fecha output file */
    fclose(outputFile);

    /* Termina o clock */
    clock_t end_t = clock();

    free_tecnicofs(fs);

    /* Gives the time and prints it */
    double total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("TecnicoFS completed in %0.4f seconds.\n", total_t);

    exit(EXIT_SUCCESS);
}
