/* Grupo 31 Taguspark */
/* Bernardo Gonçalves de Faria - número 87636 */
#define _GNU_SOURCE
#define MAXTHREADS 100
#define MAXBUFFERSIZE 300
#define MAXOPENFILES 5

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
#include "lib/inodes.h"
#include "tecnicofs-client-api.h"
#include <stdbool.h> 


typedef struct threadArgs {
    uid_t uid;
    int clientSockete;
} threadArgs;


typedef struct openFilesTable {
    int inumber;
    permission mode1;
    permission mode2;
} openFilesTable;




char* global_inputFile = NULL;
char* global_outputFile = NULL;

pthread_t tid[MAXTHREADS];

char* nomesocket;
int sockfd, socketclient, numBuckets;
int current_thread = 0;

pthread_mutex_t commandsLock;
tecnicofs* fs;
uid_t *ownerUID;

TIMER_T startTime, stopTime;


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


/**************************************
 *          Funcao trataCtrlc         *
 *************************************/

void trataCtrlC(int s) {
    for(int i = 0; i < current_thread; i++) {
        if(pthread_join(tid[i], NULL))
            exit(1);
        tfsUnmount();
    }
    /* Termina o tempo */
    TIMER_READ(stopTime);
    printf("TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    close(sockfd);
    exit(EXIT_SUCCESS);
}



/****************************************************
 *                 Funcao searchOFT                 *
 *   Procura se determinado ficheiro está na tabela *
 ****************************************************/

bool searchOFT(int fd, struct openFilesTable **table) {
    for(int i = 0; i < MAXOPENFILES; i++) {
        if(table[i]->inumber == fd) {
            return true;
        }
    }
    return false;
}


/*******************************************************
 *                 Funcao deleteFromOFT                *
 *   Remove um ficheiro da tabela de ficheiros abertos *
 *******************************************************/

int deleteFromOFT(int fd, struct openFilesTable **table) {
    for(int i = 0; i < MAXOPENFILES; i++) {
        if(table[i]->inumber == fd) {
            table[i]->inumber = -1;
            table[i]->mode1 = NONE;
            table[i]->mode2 = NONE;
            return 0;
        }
    }
    return -1;
}

/*************************************************
 *               Funcao giveMode                 *
 *  Dado um fd, retorna o modo correspondente    *
 *************************************************/

permission giveOwnerMode(int fd, struct openFilesTable **table) {
    for(int i = 0; i < MAXOPENFILES; i++) {
        if(table[i]->inumber == fd) {
            return table[i]->mode1;
        }
    }
    return NONE;
}

permission giveOthersMode(int fd, struct openFilesTable **table) {
    for(int i = 0; i < MAXOPENFILES; i++) {
        if(table[i]->inumber == fd) {
            return table[i]->mode2;
        }
    }
    return NONE;
}

/*************************************************
 *               Funcao giveInumber              *
 *  Dado um fd, retorna o inumber correspondente *
 *************************************************/

int giveInumber(int fd, struct openFilesTable **table) {
    for(int i = 0; i < MAXOPENFILES; i++) {
        if(table[i]->inumber == fd) {
            return table[i]->inumber;
        }
    }
    return -1;  // erro
}


void* applyCommands(void *arg){

    char buffer[MAXBUFFERSIZE];
    int count = 0;
    uid_t uid = ((struct threadArgs*)arg)->uid;
    // int sockete = ((struct threadArgs*)arg)->clientSockete;


    /* tabela de ficheiros abertos */
    openFilesTable **oPT;
    oPT = (struct openFilesTable**) malloc(sizeof(struct openFilesTable**) * MAXOPENFILES);
    for(int i = 0; i < MAXOPENFILES; i++) {
        oPT[i] = malloc(sizeof(openFilesTable));
        oPT[i]->inumber = -1;
        oPT[i]->mode1 = NONE;
        oPT[i]->mode2 = NONE;
    }

    while(1){

        mutex_lock(&commandsLock);

        char token, arg1[MAX_INPUT_SIZE], arg4[MAX_INPUT_SIZE];                     
        int res, arg2, arg3, lookRes, lookRes2; 

        int hashIdx = hash(arg1, numBuckets);   
        char *content = malloc((sizeof(char*)*MAXBUFFERSIZE));                            // usado no comando READ
        /* Lê do cliente */
        int n = read(socketclient, buffer, MAXBUFFERSIZE);
        if(n == 0){
            return NULL;
        }
        sscanf(buffer, "%c", &token);    

        switch (token) {
            case 'c':                                               // CREATE
                sscanf(buffer, "%c %s %d", &token, arg1, &arg2);    // c filename permissions
                mutex_unlock(&commandsLock);
                lookRes = lookup(fs, arg1, hashIdx);                // devolve inumber se existir ficheiro

                int ownerPer = arg2/10;                             // ownerPermission
                int otherPerm = arg2 % 10;                          // othersPermission

                if(lookRes == -1) {                                 // se nao existir, cria
                    res = create(fs, arg1, hashIdx, uid, ownerPer, otherPerm);      
                }
                else res = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;     // se existir, da erro
                break;
            case 'd':                                               // DELETE
                sscanf(buffer, "%c %s", &token, arg1);              // d filename
                mutex_unlock(&commandsLock);

                lookRes = lookup(fs, arg1, hashIdx);

                if(lookRes == -1) {                                  // se o ficheiro nao existir, da erro
                    res = TECNICOFS_ERROR_FILE_NOT_FOUND;
                }
                else if(searchOFT(lookRes, oPT) == true) {          // CASO O FICHEIRO ESTEJA ABERTO
                    res = TECNICOFS_ERROR_FILE_IS_OPEN;
                }
                else res = delete(fs, arg1, hashIdx, lookRes, uid);  // se existir, apaga

                printf("%d\n",res);
                break;
            case 'r':                                               // RENAME
                sscanf(buffer, "%c %s %s", &token, arg1, arg4);     // r filenameOld filenameNew
                mutex_unlock(&commandsLock);
                lookRes = lookup(fs, arg1, hashIdx);
                lookRes2 = lookup(fs, arg4, hashIdx);

                if(lookRes == -1) {
                    res = TECNICOFS_ERROR_FILE_NOT_FOUND;
                }
                else if (lookRes2 != -1) {
                    res = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
                }
                else if(searchOFT(lookRes, oPT) == true) {          // CASO O FICHEIRO ESTEJA ABERTO
                    res = TECNICOFS_ERROR_FILE_IS_OPEN;
                }
                else res = renameFile(fs, arg1, arg4, hashIdx, numBuckets, uid, lookRes);
                break;
            case 'o':                                               // OPEN
                sscanf(buffer, "%c %s %d", &token, arg1, &arg2);    // o filename mode
                mutex_unlock(&commandsLock);
                
                lookRes = lookup(fs, arg1, hashIdx);

                if (lookRes == -1) {
                    res = TECNICOFS_ERROR_FILE_NOT_FOUND;
                }

                else if (count >= 5) {
                    res = TECNICOFS_ERROR_MAXED_OPEN_FILES;
                }
                else {
                    uid_t *ownerUID = (uid_t *) malloc(sizeof(uid_t*));
                    permission *ownerPermissions = (permission *) malloc(sizeof(permission*));
                    permission *othersPermissions = (permission *) malloc(sizeof(permission*));
                    inode_get(lookRes, ownerUID, ownerPermissions, othersPermissions, NULL, 0);

                    if(uid == *ownerUID) {
                        if(arg2 == *ownerPermissions) {
                            oPT[count]->inumber = lookRes;
                            oPT[count]->mode1 = arg2;
                            count++;
                        } else if (*ownerPermissions == 3) {
                            oPT[count]->inumber = lookRes;
                            oPT[count]->mode1 = arg2;
                            count++;
                        } else {
                            res = TECNICOFS_ERROR_PERMISSION_DENIED;
                        }
                    } else {
                        if(arg2 == *othersPermissions) {
                            oPT[count]->inumber = lookRes;
                            oPT[count]->mode2 = arg2;
                            count++;
                        } else if (*othersPermissions == 3) {
                            oPT[count]->inumber = lookRes;
                            oPT[count]->mode2 = arg2;
                            count++;
                        } else {
                            res = TECNICOFS_ERROR_PERMISSION_DENIED;
                        }
                    }
                }
                break;
            case 'x':                                               // CLOSE
                sscanf(buffer, "%c %d", &token, &arg2);             // x fd
                mutex_unlock(&commandsLock);
                if (searchOFT(arg2, oPT) == false) {
                    res = TECNICOFS_ERROR_FILE_NOT_OPEN;
                }
                else {
                    res = deleteFromOFT(arg2, oPT);
                    if(res == -1) res = TECNICOFS_ERROR_OTHER;
                }
                break;
            case 'l':                                               // READ
                sscanf(buffer, "%c %d %d", &token, &arg2, &arg3);   // l fd len
                mutex_unlock(&commandsLock);

                lookRes = giveInumber(arg2, oPT);
                if(lookRes == -1) {
                    res = TECNICOFS_ERROR_FILE_NOT_OPEN;
                    break;
                }

                ownerUID = (uid_t *) malloc(sizeof(uid_t*));
                inode_get(lookRes, ownerUID, NULL, NULL, NULL, 0);

                if (*ownerUID == uid) {
                    if(giveOwnerMode(arg2, oPT) < 2) {
                        res = TECNICOFS_ERROR_INVALID_MODE;
                        break;
                    }
                } else {
                    if(giveOthersMode(arg2, oPT) < 2) {
                        res = TECNICOFS_ERROR_INVALID_MODE;
                        break;
                    }
                }
                res = readFile(arg2, arg3, content);
                if(res < 0) res = TECNICOFS_ERROR_OTHER;

                break;
            case 'w':                                               // WRITE
                sscanf(buffer, "%c %d %s", &token, &arg2, arg1);    // w fd dataInBuffer
                mutex_unlock(&commandsLock);

                lookRes = giveInumber(arg2, oPT);

                if(lookRes == -1) {
                    res = TECNICOFS_ERROR_FILE_NOT_FOUND;
                    break;
                }
                if (searchOFT(lookRes, oPT) == false) {
                    res = TECNICOFS_ERROR_FILE_NOT_OPEN;
                } else {
                    ownerUID = (uid_t *) malloc(sizeof(uid_t*));
                    inode_get(lookRes, ownerUID, NULL, NULL, NULL, 0);
                    
                    if (*ownerUID == uid) {
                        if( (giveOwnerMode(arg2, oPT) == 0) || (giveOwnerMode(arg2, oPT) == 2) ) {
                            res = TECNICOFS_ERROR_INVALID_MODE;
                        }
                    } else {
                        if( (giveOthersMode(arg2, oPT) == 0) || (giveOthersMode(arg2, oPT) == 2) ) {
                            res = TECNICOFS_ERROR_INVALID_MODE;
                        }
                    }
                
                    res = writeFile(lookRes, arg1);
                }
                
                break;
            default: { /* error */
                mutex_unlock(&commandsLock);
                fprintf(stderr, "Error: commands to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    
        if(write(socketclient, &res, sizeof(int)) < 0) {
            perror("Falhou no write");
        }
        if(content) {
            write(socketclient, content, sizeof(char) * strlen(content));         // para ler o conteudo no READ
        }
    }
    puts("sai do apply");
    return NULL;
}





int main(int argc, char* argv[]) {
    parseArgs(argc, argv);

    /* Estruturas e variáveis */
    int servlen;
    struct sockaddr_un serverAddress; 
    struct ucred credentials;


    /* Verifica se ocorreu ctrl+c */
    signal(SIGINT, trataCtrlC);


    /* nome do socket */
    nomesocket = argv[1];

    /* numero de buckets */
    numBuckets = atoi(argv[3]);   


    
    fs = new_tecnicofs(numBuckets);
    mutex_init(&commandsLock);
    inode_table_init();


    /* criacao do socket do servidor */
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    /* remove caso haja lixo */
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

    /* Começa o tempo */
    TIMER_READ(startTime);

    /* ciclo para aceitar pedidos */
    for(;;) {


        /* aceita ligacao */
        if((socketclient = accept(sockfd, NULL, NULL)) < 0) {
            perror("Erro ao criar ligacao - accept");
        }

        /* saber qual o cliente a fazer a conecao */
        socklen_t len = sizeof(struct ucred);   
        if(getsockopt(socketclient, SOL_SOCKET, SO_PEERCRED, &credentials, &len) == -1) { // queremos o user id (UID)
            perror("getsockopt failed");
        }

        /* Alocacao da estrutura passada na thread */
        struct threadArgs *Allen = (struct threadArgs *)malloc(sizeof(struct threadArgs));
        Allen->uid = credentials.uid;
        Allen->clientSockete = socketclient;

        /* Criacao das threads */
        if(pthread_create(&tid[current_thread++], NULL, applyCommands, (void *)Allen) != 0 ) {
           printf("Failed to create thread\n");
        }

        
    }

    FILE * outputFp = openOutputFile();

    print_tecnicofs_tree(outputFp, fs, numBuckets);
    fflush(outputFp);
    fclose(outputFp);

    mutex_destroy(&commandsLock);
    inode_table_destroy();
    free_tecnicofs(fs, numBuckets);
    
    exit(EXIT_SUCCESS);
}

