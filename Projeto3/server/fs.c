/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#define NAMEFILESZ 100

#include "fs.h"
#include "lib/bst.h"
#include "lib/hash.h"
#include "lib/inodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sync.h"
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


/* Variaveis para usar o inode_get */
uid_t *ownerUID;
permission *ownerPermissions;
permission *othersPermissions;
char* content;
int lenContent;


/****************
* new_tecnicofs *
*****************/
tecnicofs* new_tecnicofs(int numBuckets){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->nextINumber = 0;
	fs->bstRoot = malloc(sizeof(node **)*numBuckets);		
	fs->bstLock = malloc(sizeof(syncMech)*numBuckets);		

	for(int i = 0; i < numBuckets; i++) {					
		fs->bstRoot[i] = NULL;								
		sync_init(&(fs->bstLock[i]));
	}
	return fs;
}

/*****************
* free_tecnicofs *
******************/
void free_tecnicofs(tecnicofs* fs, int numBuckets) {
	for(int i = 0; i < numBuckets; i++) {
		free_tree(fs->bstRoot[i]);
		sync_destroy(&(fs->bstLock[i]));
	}
	free(fs);
}


/*********
* Create *
**********/
int create(tecnicofs* fs, char *name, int hashIdx, uid_t owner, permission ownerPerm, permission othersPerm){
	sync_wrlock(&(fs->bstLock[hashIdx]));
	int inumber = inode_create(owner, ownerPerm, othersPerm);				// cria um inode
	fs->bstRoot[hashIdx] = insert(fs->bstRoot[hashIdx], name, inumber);
	sync_unlock(&(fs->bstLock[hashIdx]));
	return inumber;
}



/*********
* Delete *
**********/
int delete(tecnicofs* fs, char *name, int hashIdx, int inumberLook, uid_t uid){
	sync_wrlock(&(fs->bstLock[hashIdx]));

	int res;
	ownerUID = (uid_t *) malloc(sizeof(uid_t*));

	inode_get(inumberLook, ownerUID, NULL, NULL, NULL, 0);		// retorna o ownerUID
	
	if((long)uid != (long)*ownerUID) {							// se o cliente nao for o owner, da erro
		res = TECNICOFS_ERROR_OTHER;
		sync_unlock(&(fs->bstLock[hashIdx]));
	}

	else {															     //se for, apaga o ficheiro da bst 
		fs->bstRoot[hashIdx] = remove_item(fs->bstRoot[hashIdx], name);  // apaga o ficheiro da bst
		res = inode_delete(inumberLook);								 // e da tabela de inodes
		if(res != 0) res = TECNICOFS_ERROR_OTHER;
		sync_unlock(&(fs->bstLock[hashIdx]));
	}
	return res;
}



/*********
* Lookup *
**********/
int lookup(tecnicofs* fs, char *name, int hashIdx){
	sync_rdlock(&(fs->bstLock[hashIdx]));
	int inumber = -1;
	node* searchNode = search(fs->bstRoot[hashIdx], name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock[hashIdx]));
	return inumber;
}



/*********
* Rename *
**********/
int renameFile(tecnicofs* fs, char* actualName, char* newName, int hashIdxName, int numBuckets, uid_t uid, int inumberOld) {
	sync_wrlock(&(fs->bstLock[hashIdxName]));

	int hashIdxNewName = hash(newName, numBuckets);		
	int res;

	ownerUID = (uid_t *) malloc(sizeof(uid_t*));
	inode_get(inumberOld, ownerUID, NULL, NULL, NULL, 0);		// retorna o ownerUID do ficheiro

	if((long)uid != (long)*ownerUID) {							// se o cliente nao for o owner, da erro
		res = TECNICOFS_ERROR_PERMISSION_DENIED;
		sync_unlock(&(fs->bstLock[hashIdxName]));
	}

	else {
		node* searchName = search(fs->bstRoot[hashIdxName], actualName);				// procura o node a ser mudado
		node* searchNewName = search(fs->bstRoot[hashIdxNewName], newName);				// procura o node que vai substituir o acima

		if(searchName != NULL && searchNewName == NULL) {						// se o primeiro for encontrado e o segundo nao, entao faz o rename
			int iNumber = fs->bstRoot[hashIdxName]->inumber;					// guarda o inumber do primeiro

			fs->bstRoot[hashIdxName] = remove_item(fs->bstRoot[hashIdxName], actualName);					// remove o primeiro node
			fs->bstRoot[hashIdxNewName] = insert(fs->bstRoot[hashIdxNewName], newName, iNumber);			// substitui pelo novo
		}
		sync_unlock(&(fs->bstLock[hashIdxName]));
		res = 0;
	}
	return res;
}


/*************************************************
* getPerm - transforma um inteiro numa permissao *
**************************************************/
permission getPerm(int perm) {
	permission p;
	if (perm == 1) return p = WRITE;
	else if (perm == 2) return p = READ;
	else if (perm == 3) return p = RW;
	else return p = NONE;
}


/***********
* readFile *
************/
int readFile(int inumber, int bufferLen, char* fContent) {

	int res;

	inode_get(inumber, NULL, NULL, NULL, fContent, bufferLen-1);	// le o conteudo do ficheiro (len-1)
	strcat(fContent, "\0");											// adiciona depois o \0
	return res = strlen(fContent);
}


/************
* writeFile *
*************/
int writeFile(int inumber, char* dataInBuffer) {

	int res;
	int len = strlen(dataInBuffer);
	int resultado = inode_set(inumber, dataInBuffer, len);			// atualiza o conteudo do ficheiro

	if(resultado == -1) {
		return res = TECNICOFS_ERROR_OTHER;
	}
	else return res = 0;
	
}

/************************
* printf_tecnicofs_tree *
*************************/
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int numBuckets){
	for(int i = 0; i < numBuckets; i++) {
		print_tree(fp, fs->bstRoot[i]);
	}
}
