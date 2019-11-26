/* Grupo 31 Taguspark */
/* Bernardo Gonçalves de Faria - número 87636 */
#define _GNU_SOURCE
#define MAXTHREADS 100
#define MAXBUFFERSIZE 140

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <pthread.h>
#include "fs.h"
#include "constants.h"
#include "lib/hash.h"
#include "lib/timer.h"
#include "sync.h"
#include <signal.h>

char* global_inputFile = NULL;
char* global_outputFile = NULL;

int numBuckets;
char* nomesocket;
int sockfd, socketclient;

pthread_mutex_t commandsLock;
tecnicofs* fs;


static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {                                        // passa a receber outro input
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}


void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
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


// void trataCtrlC(int s) {
//   char c;
//   printf("Queres mesmo terminar?\n");
//   c = getc(stdin);
//   if (c=='s')
//     exit(EXIT_SUCCESS);
//   else
//     signal(SIGINT, trataCtrlC);
// }



void* applyCommands(){

    char buffer[MAXBUFFERSIZE];

    while(1){
        mutex_lock(&commandsLock);
        char token;                                     // letra identificadora de qual comando executar
        char arg1[MAX_INPUT_SIZE];                      // segundo argumento
        char arg2[MAX_INPUT_SIZE];                      // terceiro argumento  
        int iNumber;
        int hashIdx = hash(arg1, numBuckets);           // valor de hash
        int res;                                        // return da funcao se devolver int
        char *content = NULL;                           // return da funcao se devolver string

        /* Lê do cliente */
        read(socketclient, buffer, MAXBUFFERSIZE);
        
        sscanf(buffer, "%c %s %s", &token, arg1, arg2);    // adicionado

        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);
                mutex_unlock(&commandsLock);
                create(fs, arg1, iNumber, hashIdx);                    // novo argumento
                break;
            case 'l':
                mutex_unlock(&commandsLock);
                int searchResult = lookup(fs, arg1, hashIdx);          // novo argumento
                if(!searchResult)
                    printf("%s not found\n", arg1);
                else
                    printf("%s found with inumber %d\n", arg1, searchResult);
                break;
            case 'd':
                mutex_unlock(&commandsLock);
                delete(fs, arg1, hashIdx);
                break;
            case 'r':
                mutex_unlock(&commandsLock);
                renameFile(fs, arg1, arg2, hashIdx, numBuckets);     // novo comando
                break;
            case 'o':
                mutex_unlock(&commandsLock);
                printf("Cheguei ao 'o'");
            case 'x':
                mutex_unlock(&commandsLock);
                printf("Cheguei ao 'x'");
            case 'w':
                mutex_unlock(&commandsLock);
                printf("cheguei ao 'w'");
            default: { /* error */
                mutex_unlock(&commandsLock);
                fprintf(stderr, "Error: commands to apply\n");
                exit(EXIT_FAILURE);
            }
        }

    write(socketclient, &res, sizeof(int));
    if(content) write(socketclient, content, sizeof(char)*res);

    }  
    return NULL;
}





int main(int argc, char* argv[]) {
    parseArgs(argc, argv);

    struct sockaddr_un serverAddress, clientAddress; 
    int servlen, clientDimension, childpid, i;
    struct ucred credentials;

    /* le nome do socket */
    nomesocket = argv[1];

    /* le numero de buckets */
    numBuckets = atoi(argv[3]);   


    /* criacao do socket do servidor */
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    unlink(nomesocket);

    bzero((char *)&serverAddress, sizeof(serverAddress)); 
    serverAddress.sun_family= AF_UNIX;
    strcpy(serverAddress.sun_path, nomesocket);
    servlen= strlen(serverAddress.sun_path) + sizeof(serverAddress.sun_family);
    if (bind(sockfd, (struct sockaddr_un *) &serverAddress, servlen) < 0) {
        perror("erro ao atribuir nome socket servidor");
    }

    /* Disponibilidade para esperar pedidos de ligacoes */
    listen(sockfd, 100);

    /* ciclo para aceitar pedidos */
    for(;;) {
    
        /* aceita ligacao */
        if((socketclient = accept(sockfd, NULL, NULL)) < 0) {
            perror("Erro ao criar ligacao - accept");
        }

        /* Verifica se ocorreu ctrl+c */
        // signal(SIGINT, trataCtrlC);

        int len = sizeof(struct ucred);   
        if(getsockopt(socketclient, SOL_SOCKET, SO_PEERCRED, &credentials, &len) == -1) { // queremos o user id (UID)
            perror("getsockopt failed");
        }

        printf("pid=%ld, euid=%ld, egid=%ld\n",(long) credentials.pid, (long) credentials.uid, (long) credentials.gid);
        
        pthread_t tid[MAXTHREADS];
        
        if(pthread_create(&tid[i++], NULL, applyCommands, &socketclient) != 0 ) {
           printf("Failed to create thread\n");
        }

        
    }


    FILE * outputFp = openOutputFile();
    mutex_init(&commandsLock);
    fs = new_tecnicofs(numBuckets);

    print_tecnicofs_tree(outputFp, fs, numBuckets);
    fflush(outputFp);
    fclose(outputFp);

    mutex_destroy(&commandsLock);

    free_tecnicofs(fs, numBuckets);
    
    exit(EXIT_SUCCESS);
}

