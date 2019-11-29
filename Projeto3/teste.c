#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <assert.h>

#define MAXLEN 140

int sockfd;
int buffer;

/****************************************************
 * 
 * SAO PRECISOS \0 NO FINAL DE CADA STR????????????
 * 
 * *************************************************/


int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {

    char str[MAXLEN];
    sprintf(str, "c %s %d%d%c", filename, ownerPermissions, othersPermissions, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if(buffer != TECNICOFS_ERROR_FILE_ALREADY_EXISTS) return 0;
    else return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
}


int tfsDelete(char *filename) {

    char str[MAXLEN];
    sprintf(str, "d %s", filename);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}





int tfsRename(char *filenameOld, char *filenameNew) {
    
    char str[MAXLEN];
    sprintf(str, "r %s %s", filenameOld,filenameNew);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}




int tfsOpen(char *filename, permission mode) {

    char str[MAXLEN];
    sprintf(str, "o %s %d", filename, mode);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}



int tfsClose(int fd) {
    
    char str[MAXLEN];
    sprintf(str, "x %d", fd);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}




int tfsRead(int fd, char *buffer, int len) {
    
    char str[MAXLEN];
    sprintf(str, "l %d %d", fd, len);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}




int tfsWrite(int fd, char *buffer, int len) {

    char str[MAXLEN];
    sprintf(str, "w %d %s", fd, buffer);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}



int tfsMount(char * address) {
    int servlen;
    struct sockaddr_un serv_addr;

    /* Cria socket stream */
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        perror("client: can't open stream socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family= AF_UNIX;
    strcpy(serv_addr.sun_path, address);
    servlen= strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    if(connect(sockfd,(struct sockaddr *) &serv_addr, servlen) < 0)
        // perror("client:can't connect to server");
        return TECNICOFS_ERROR_OPEN_SESSION;

    return 0;
}



int tfsUnmount() {

    if(close(sockfd) == 0) {
        return 0;
    }
    else return TECNICOFS_ERROR_NO_OPEN_SESSION;
}



int main(int argc, char** argv) {
     if (argc != 2) {
        printf("Usage: %s sock_path\n", argv[0]);
        exit(0);
    }
    assert(tfsMount(argv[1]) == 0);

    printf("Test: create file sucess\n");
    assert(tfsCreate("a", RW, READ) == 0);
    printf("Sucesso\n");

    printf("Test2: create file sucess\n");
    assert(tfsCreate("b", RW, READ) == 0);
    printf("Sucess2\n");

    printf("Test3: create file sucess\n");
    assert(tfsCreate("c", RW, READ) == 0);
    printf("Sucess3\n");

    // printf("Test: delete file success\n");
    // assert(tfsDelete("a") == 0);
    // printf("Sucesso3\n");

    // printf("Test: delete non existing file success\n");
    // assert(tfsDelete("fabio") == -5);
    // printf("Sucesso4\n");

    assert(tfsUnmount() == 0);

    return 0;
}

