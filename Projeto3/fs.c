/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include "fs.h"
#include "lib/bst.h"
#include "lib/hash.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sync.h"
#include "lib/inodes.h"
#include <sys/types.h>


/* Variaveis para usar o inode_get */
uid_t *ownerUID;
permission *ownerPermissions;
permission *othersPermissions;
char* content;
int lenContent;


int obtainNewInumber(tecnicofs* fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

tecnicofs* new_tecnicofs(int numBuckets){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->nextINumber = 0;
	fs->bstRoot = malloc(sizeof(node **)*numBuckets);		// malloc da hashtable
	fs->bstLock = malloc(sizeof(syncMech)*numBuckets);		// malloce dos mecanismos de sincronizacao

	for(int i = 0; i < numBuckets; i++) {					// for que mete cada node da hash table a null
		fs->bstRoot[i] = NULL;								// e inicializa um lock para cada bst
		sync_init(&(fs->bstLock[i]));
	}
	return fs;
}




void free_tecnicofs(tecnicofs* fs, int numBuckets) {
	for(int i = 0; i < numBuckets; i++) {
		free_tree(fs->bstRoot[i]);
		sync_destroy(&(fs->bstLock[i]));
	}
	free(fs);
}


/* Modificado: passa também a criar um novo inode */

int create(tecnicofs* fs, char *name, int hashIdx, uid_t owner, permission ownerPerm, permission othersPerm){
	sync_wrlock(&(fs->bstLock[hashIdx]));
	int inumber = inode_create(owner, ownerPerm, othersPerm);
	fs->bstRoot[hashIdx] = insert(fs->bstRoot[hashIdx], name, inumber);
	sync_unlock(&(fs->bstLock[hashIdx]));
	return inumber;  // pode ser -1
}




int delete(tecnicofs* fs, char *name, int hashIdx, int inumberLook, uid_t uid){
	sync_wrlock(&(fs->bstLock[hashIdx]));

	int res;
	ownerUID = (uid_t *) malloc(sizeof(uid_t*));

	inode_get(inumberLook, ownerUID, NULL, NULL, NULL, 0);
	
	if((long)uid != (long)*ownerUID) {
		res = TECNICOFS_ERROR_OTHER;
		sync_unlock(&(fs->bstLock[hashIdx]));
	}

	else {
		fs->bstRoot[hashIdx] = remove_item(fs->bstRoot[hashIdx], name);
		res = inode_delete(inumberLook);
		if(res != 0) res = TECNICOFS_ERROR_OTHER;
		sync_unlock(&(fs->bstLock[hashIdx]));
	}
	return res;
}




int lookup(tecnicofs* fs, char *name, int hashIdx){
	sync_rdlock(&(fs->bstLock[hashIdx]));
	int inumber = -1;
	// int inumber = 0;
	node* searchNode = search(fs->bstRoot[hashIdx], name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock[hashIdx]));
	return inumber;
}




/* Funcao de rename de um ficheiro */

int renameFile(tecnicofs* fs, char* actualName, char* newName, int hashIdxName, int numBuckets, uid_t uid, int inumberOld) {
	sync_wrlock(&(fs->bstLock[hashIdxName]));

	int hashIdxNewName = hash(newName, numBuckets);		// valor de hash do novo nome
	int res;

	ownerUID = (uid_t *) malloc(sizeof(uid_t*));
	inode_get(inumberOld, ownerUID, NULL, NULL, NULL, 0);

	if((long)uid != (long)*ownerUID) {
		res = TECNICOFS_ERROR_PERMISSION_DENIED;
		sync_unlock(&(fs->bstLock[hashIdxName]));
	}

	else {
		node* searchName = search(fs->bstRoot[hashIdxName], actualName);				// procura o node a ser mudado
		node* searchNewName = search(fs->bstRoot[hashIdxNewName], newName);		// procura o node que vai substituir o acima

		if(searchName != NULL && searchNewName == NULL) {						// se o primeiro for encontrado e o segundo nao, entao faz o rename
			int iNumber = fs->bstRoot[hashIdxName]->inumber;					// guarda o inumber do primeiro

			fs->bstRoot[hashIdxName] = remove_item(fs->bstRoot[hashIdxName], actualName);					// remove o primeiro node
			fs->bstRoot[hashIdxNewName] = insert(fs->bstRoot[hashIdxNewName], newName, iNumber);	// substitui pelo novo
		}
		sync_unlock(&(fs->bstLock[hashIdxName]));
		res = 0;
	}
	return res;
}





void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int numBuckets){
	for(int i = 0; i < numBuckets; i++) {
		print_tree(fp, fs->bstRoot[i]);
	}
}
