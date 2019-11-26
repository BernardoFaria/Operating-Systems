#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <sys/un.h>
#include <sys/types.h>          
#include <sys/socket.h>

int sockfd, buffer;


int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {

    char str = sprintf("c %s %d %d\0", filename, ownerPermissions, othersPermissions);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;

}




int tfsDelete(char *filename) {

    char str = sprintf("d %s\0", filename);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}





int tfsRename(char *filenameOld, char *filenameNew) {
    
    char str = sprintf("r %s %s\0", filenameOld,filenameNew);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));

    return 0;
}




int tfsOpen(char *filename, permission mode) {

    char str = sprintf("o %s %d\0", filename, mode);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}



int tfsClose(int fd) {
    
    char str = sprintf("x %d\0", fd);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}




int tfsRead(int fd, char *buffer, int len) {
    
    char str = sprintf("l %d %d\0", fd, len);

    write(sockfd, str, sizeof(char)*strlen(str));
    read(sockfd, &buffer, sizeof(int));
    return 0;
}




int tfsWrite(int fd, char *buffer, int len) {

    char str = sprintf("w %d %s\0", fd, buffer);

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


