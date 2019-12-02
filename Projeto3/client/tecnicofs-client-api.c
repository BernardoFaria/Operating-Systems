#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <assert.h>

#define MAXLEN 100000

int sockfd;
int buffer;

/*********
* Create *
**********/
int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {

    char str[MAXLEN];
    sprintf(str, "c %s %d%d%c", filename, ownerPermissions, othersPermissions, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if(buffer != TECNICOFS_ERROR_FILE_ALREADY_EXISTS) return 0;
    else return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
}


/*********
* Delete *
**********/
int tfsDelete(char *filename) {

    char str[MAXLEN];
    sprintf(str, "d %s%c", filename, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if(buffer == TECNICOFS_ERROR_FILE_NOT_FOUND) return TECNICOFS_ERROR_FILE_NOT_FOUND;
    else if(buffer == TECNICOFS_ERROR_OTHER) return TECNICOFS_ERROR_OTHER;
    else if(buffer == TECNICOFS_ERROR_FILE_IS_OPEN) return TECNICOFS_ERROR_FILE_IS_OPEN;
    else return 0;
}


/*********
* Rename *
**********/
int tfsRename(char *filenameOld, char *filenameNew) {
    
    char str[MAXLEN];
    sprintf(str, "r %s %s%c", filenameOld,filenameNew, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if(buffer == TECNICOFS_ERROR_FILE_NOT_FOUND) return TECNICOFS_ERROR_FILE_NOT_FOUND;
    else if(buffer == TECNICOFS_ERROR_FILE_ALREADY_EXISTS) return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    else if(buffer == TECNICOFS_ERROR_PERMISSION_DENIED) return TECNICOFS_ERROR_PERMISSION_DENIED;
    else if(buffer == TECNICOFS_ERROR_FILE_IS_OPEN) return TECNICOFS_ERROR_FILE_IS_OPEN;
    else return 0;
}


/*********
*  Open  *
**********/
int tfsOpen(char *filename, permission mode) {

    char str[MAXLEN];
    sprintf(str, "o %s %d%c", filename, mode, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if (buffer == TECNICOFS_ERROR_FILE_NOT_FOUND) return TECNICOFS_ERROR_FILE_NOT_FOUND;
    else if (buffer == TECNICOFS_ERROR_MAXED_OPEN_FILES) return TECNICOFS_ERROR_MAXED_OPEN_FILES;
    else if (buffer == TECNICOFS_ERROR_PERMISSION_DENIED) return TECNICOFS_ERROR_PERMISSION_DENIED;
    else return 0;
}


/*********
* Close  *
**********/
int tfsClose(int fd) {
    
    char str[MAXLEN];
    sprintf(str, "x %d%c", fd, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if (buffer == TECNICOFS_ERROR_FILE_NOT_OPEN) return TECNICOFS_ERROR_FILE_NOT_OPEN;
    else if (buffer == TECNICOFS_ERROR_OTHER) return TECNICOFS_ERROR_OTHER;
    else return 0;
}


/*********
*  Read  *
**********/
int tfsRead(int fd, char *buff, int len) {
    
    char str[MAXLEN];
    sprintf(str, "l %d %d%c", fd, len, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, (sizeof(int)));                 
    if (buffer == TECNICOFS_ERROR_FILE_NOT_OPEN) return TECNICOFS_ERROR_FILE_NOT_OPEN;
    else if (buffer == TECNICOFS_ERROR_INVALID_MODE) return TECNICOFS_ERROR_INVALID_MODE;
    else if(buffer == TECNICOFS_ERROR_OTHER) return TECNICOFS_ERROR_OTHER;
    else {
        read(sockfd, buff, (sizeof(char*))*buffer);           // le a string recebida
        return buffer;
    }
}


/*********
* Write  *
**********/
int tfsWrite(int fd, char *buff, int len) {

    char str[MAXLEN];
    char res[len];
    
    strncpy(res, buff, len);
    sprintf(str, "w %d %s%c", fd, res, '\0');

    write(sockfd, str, sizeof(char)*(strlen(str)+1));
    read(sockfd, &buffer, sizeof(int));
    if (buffer == TECNICOFS_ERROR_INVALID_MODE) return TECNICOFS_ERROR_INVALID_MODE;
    else if (buffer == TECNICOFS_ERROR_FILE_NOT_OPEN) return TECNICOFS_ERROR_FILE_NOT_OPEN;
    else if (buffer == TECNICOFS_ERROR_OTHER) return TECNICOFS_ERROR_OTHER;
    else return 0;
}


/*********
* Mount  *
**********/
int tfsMount(char * address) {
    int servlen;
    struct sockaddr_un serv_addr;

    /* Cria socket stream */
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        perror("Erro");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family= AF_UNIX;
    strcpy(serv_addr.sun_path, address);
    servlen= strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    if(connect(sockfd,(struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("Erro");
        return TECNICOFS_ERROR_OPEN_SESSION;
    }

    return 0;
}


/**********
* Unmount *
***********/
int tfsUnmount() {
    if(close(sockfd) == 0) {
        return 0;
    }
    else {
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
}
