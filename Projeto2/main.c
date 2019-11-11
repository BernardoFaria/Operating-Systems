/* Grupo 31 Taguspark */
/* Bernardo Gonçalves de Faria - número 87636 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "fs.h"
#include "constants.h"
#include "lib/hash.h"
#include "lib/timer.h"
#include "sync.h"
#include <semaphore.h>

char* global_inputFile = NULL;
char* global_outputFile = NULL;
int numberThreads = 0;
int numBuckets = 0;                 //numero de buckets
pthread_mutex_t commandsLock;
tecnicofs* fs;

sem_t prod, cons;                   // semaforos  


char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 5) {                                        // passa a receber outro input
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    global_inputFile = argv[1];
    global_outputFile = argv[2];
    numberThreads = atoi(argv[3]);
    if (!numberThreads) {
        fprintf(stderr, "Invalid number of threads\n");
        displayUsage(argv[0]);
    }
}



int insertCommand(char* data) {
    strcpy(inputCommands[numberCommands++], data);
    if(numberCommands == MAX_COMMANDS)              // se existirem 10 comandos no vetor,
        numberCommands = 0;                         // volta a escrever no inicio
    return 1;
}



char* removeCommand() {
    char * command = inputCommands[headQueue++];
    if(headQueue == MAX_COMMANDS)                   // se a headQueue estiver cheia,
        headQueue = 0;                              // mete-a a 0 
    return command;
}



void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}


void *processInput(){
    FILE* inputFile;
    inputFile = fopen(global_inputFile, "r");
    if(!inputFile){
        fprintf(stderr, "Error: Could not read %s\n", global_inputFile);
        exit(EXIT_FAILURE);
    }
    char line[MAX_INPUT_SIZE];
    int lineNumber = 0;

    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token;
        char name[MAX_INPUT_SIZE];
        char newName[MAX_INPUT_SIZE];           // novo nome
        lineNumber++;

        int numTokens = sscanf(line, "%c %s %s", &token, name, newName);       // adicionado um argumento

        // if(numTokens)

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
            case 'l':
            case 'd':
                if(numTokens != 2)
                    errorParse(lineNumber);
                sem_wait(&prod);                // espera para o produtor
                if(insertCommand(line)) {
                    sem_post(&cons);            // sinal para o consumidor
                    break;
                }
                return NULL;
                break;
            case 'r':                       // novo comando
                if(numTokens != 3)
                    errorParse(lineNumber);
                sem_wait(&prod);                // espera para o produtor
                if(insertCommand(line)) {
                    sem_post(&cons);            // sinal para o consumidor
                    break;
                }
                return NULL;
                break;
            case '#':
                break;
            default: { /* error */
                errorParse(lineNumber);
            }
        }

    }

    sem_wait(&prod);            // espera para o produtor
    if(insertCommand("f")) {    // mete um 'f' no final para avisar uma thread que vai sair
        sem_post(&cons);        // sinal para o consumidor
    }

    fclose(inputFile);
    return NULL;
}



FILE * openOutputFile() {
    FILE *fp;
    fp = fopen(global_outputFile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}



void*  applyCommands(){
    while(1){
        sem_wait(&cons);                   // wait para os consumidores
        mutex_lock(&commandsLock);
        const char* command = removeCommand();
        if (command == NULL){
            mutex_unlock(&commandsLock);
            continue;                  
        }

        char token;
        char name[MAX_INPUT_SIZE];
        char newName[MAX_INPUT_SIZE];                          // novo nome para o comando 'r'
        
        sscanf(command, "%c %s %s", &token, name, newName);    // adicionado
        sem_post(&prod);                                       // assinala os produtores
        
        int iNumber;
        int hashIdx = hash(name, numBuckets);           // valor de hash

        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);
                mutex_unlock(&commandsLock);
                create(fs, name, iNumber, hashIdx);                    // novo argumento
                break;
            case 'l':
                mutex_unlock(&commandsLock);
                int searchResult = lookup(fs, name, hashIdx);          // novo argumento
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                mutex_unlock(&commandsLock);
                delete(fs, name, hashIdx);
                break;
            case 'r':
                mutex_unlock(&commandsLock);
                renameFile(fs, name, newName, hashIdx, numBuckets);     // novo comando
                break;
            case 'f':
                sem_wait(&prod);            // espera para os produtores
                if(insertCommand("f")) {    // uma thread lè o 'f' enfiado pelo produtor, e sempre que essa thread sai, adiciona um 'f' para a proxima sair
                   sem_post(&cons);         // sinal para os consumidores
                }
                mutex_unlock(&commandsLock);
                return NULL; 
            default: { /* error */
                mutex_unlock(&commandsLock);
                fprintf(stderr, "Error: commands to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }  
    return NULL;
}



void runThreads(FILE* timeFp){
    TIMER_T startTime, stopTime;

    /* Começa o tempo */
    TIMER_READ(startTime);

    /* criação e lançamento da tarefa produtora */
    pthread_t producer;

    if(pthread_create(&producer, NULL, processInput, NULL)) {
        perror("Erro na criação da thread producer\n");
        exit(EXIT_FAILURE);
    }

    /* criação e lançamento das tarefas escravas */
    pthread_t tidConsum[numberThreads];

    for(int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tidConsum[i], NULL, applyCommands, NULL)) {
            perror("Erro na criação das threads consumidoras\n");
            exit(EXIT_FAILURE);
        }
    }

    /* join das consumidoras */
    for(int i = 0; i < numberThreads; i++) {
        if(pthread_join(tidConsum[i], NULL)) {
            perror("Can't join tidConsum threads");
        }
    }

    /* join da produtora */
    if(pthread_join(producer, NULL)) {
        perror("Can't join producer thread");
    }

    TIMER_READ(stopTime);

    fprintf(timeFp, "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));

}



    /* Funcao de inicializacao dos semaforos */
void semaphores_init() {
    if(sem_init(&prod, 0, MAX_COMMANDS) != 0) {
        perror("sem_init prod failed\n");
        exit(EXIT_FAILURE);
    }

    else if(sem_init(&cons, 0, 0) != 0) {
        perror("sem_init cons failed\n");
        exit(EXIT_FAILURE);
    }
}





int main(int argc, char* argv[]) {
    parseArgs(argc, argv);


    /* le numero de threads consumidoras */
    numberThreads = atoi(argv[3]);

    /* le numero de buckets */
    numBuckets = atoi(argv[4]);         


    FILE * outputFp = openOutputFile();
    semaphores_init();          // inicializacao dos semaforos
    mutex_init(&commandsLock);
    fs = new_tecnicofs(numBuckets);

    runThreads(stdout);
    print_tecnicofs_tree(outputFp, fs, numBuckets);
    fflush(outputFp);
    fclose(outputFp);

    mutex_destroy(&commandsLock);
    sem_destroy(&prod);             // destroy do semaforo dos produtores
    sem_destroy(&cons);             // destroy do semaforo dos consumidores

    free_tecnicofs(fs, numBuckets);
    
    exit(EXIT_SUCCESS);
}

