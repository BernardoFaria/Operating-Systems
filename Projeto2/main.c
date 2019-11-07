/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

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

hashtable_t* hashtable;             // hashtable
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
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        sem_post(&cons);
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

void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}

void *processInput(){
    FILE* inputFile;
    inputFile = fopen(global_inputFile, "r");
    if(!inputFile){
        fprintf(stderr, "Error: Could not read %s\n", global_inputFile);
        // sem_post(&cons);
        exit(EXIT_FAILURE);
    }
    char line[MAX_INPUT_SIZE];
    int lineNumber = 0;

    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        // sem_wait(&prod);        // espera do produtor para poder avançar
        char token;
        char name[MAX_INPUT_SIZE];
        lineNumber++;

        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
            case 'l':
            case 'd':
            case 'r':                       // novo comando
                if(numTokens != 2)
                    errorParse(lineNumber);
                if(insertCommand(line))
                    break;
                return NULL;
                break;
            case '#':
                break;
            default: { /* error */
                errorParse(lineNumber);
            }
        }
    }
    // sem_post(&cons);
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

void* applyCommands(){
    while(1){
        // sem_wait(&cons);                // faz esperar a tarefa consumidora
        mutex_lock(&commandsLock);
        if(numberCommands > 0){
            const char* command = removeCommand();
            if (command == NULL){
                mutex_unlock(&commandsLock);
                // sem_post(&prod);        // assinala os produtores para avançarem
                continue;
            }
            char token;
            char name[MAX_INPUT_SIZE];
            char newName[MAX_INPUT_SIZE];                        // novo nome para o comando 'r'
            sscanf(command, "%c %s %s", &token, name, newName);  // adicionado

            int iNumber;

            switch (token) {
                case 'c':
                    // printf("iNumber: %d\n", iNumber);
                    // iNumber = obtainNewInumber(hashtable->hTable[hashIdx]->fs);
                    iNumber = obtainNewInumber(fs);
                    mutex_unlock(&commandsLock);
                    // sem_post(&prod);                                    // assinala os produtores para avançarem
                    create(fs, name, iNumber);
                    // hashInsert(hashtable, hashIdx, name, iNumber);
                    break;
                case 'l':
                    mutex_unlock(&commandsLock);
                    // sem_post(&prod);                     // assinala os produtores para avançarem
                    // int searchResult = lookup(hashtable->hTable[hashIdx]->fs, name);
                    int searchResult = lookup(fs, name);
                    if(!searchResult)
                        printf("%s not found\n", name);
                    else
                        printf("%s found with inumber %d\n", name, searchResult);
                    break;
                case 'd':
                    mutex_unlock(&commandsLock);
                    // sem_post(&prod);                // assinala os produtores para avançarem
                    delete(fs, name);
                    // hashDelete(hashtable, hashIdx, name);
                    break;
                case 'r':
                    mutex_unlock(&commandsLock);
                    //renameFile(fs, name, newName);
                    break;
                default: { /* error */
                    mutex_unlock(&commandsLock);
                    // sem_post(&prod);                                // assinala os produtores para avançarem
                    fprintf(stderr, "Error: commands to apply\n");
                    puts("default");
                    exit(EXIT_FAILURE);
                }
            }
        }else{
            mutex_unlock(&commandsLock);
            // sem_post(&prod);
            return NULL;
        }
    }
    puts("terminei");   
}



void runThreads(FILE* timeFp){
    TIMER_T startTime, stopTime;

    /* Começa o tempo */
    TIMER_READ(startTime);

    /* criação e lançamento da tarefa produtora */
    pthread_t producer; // = (pthread_t*) malloc(sizeof(pthread_t));

    if(pthread_create(&producer, NULL, processInput, NULL)) {
        perror("Erro na criação da thread producer\n");
        exit(EXIT_FAILURE);
    }
    printf("Thread producer criada\n");

    /* criação e lançamento das tarefas escravas */
    pthread_t tidConsum[numberThreads]; //= (pthread_t*) malloc(numberThreads*sizeof(pthread_t));

    for(int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tidConsum[i], NULL, applyCommands, NULL)) {
            perror("Erro na criação das threads consumidoras\n");
            exit(EXIT_FAILURE);
        }
        printf("Thread consumidora %3d criada\n", i);
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
    if(sem_init(&prod, 0, MAX_INPUT_SIZE) != 0) {
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

    semaphores_init();          // inicializacao dos semaforos

    /* le numero de threads consumidoras */
    numberThreads = atoi(argv[3]);

    /* le numero de buckets */
    numBuckets = atoi(argv[4]);         

    /* cria hash table */
    hashtable = hashCreate(numBuckets);



    // processInput();

    FILE * outputFp = openOutputFile();
    mutex_init(&commandsLock);
    fs = new_tecnicofs();

    runThreads(stdout);
    // print_tecnicofs_tree(outputFp, fs);
    fflush(outputFp);
    fclose(outputFp);

    mutex_destroy(&commandsLock);
    sem_destroy(&prod);             // destroy do semaforo dos produtores
    sem_destroy(&cons);             // destroy do semaforo dos consumidores

    // free_tecnicofs(fs);
    // hashFree(hashtable);
    
    exit(EXIT_SUCCESS);
}

