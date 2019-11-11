/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include "fs.h"
#include "lib/bst.h"
#include "lib/hash.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sync.h"



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




// void create(tecnicofs* fs, char *name, int inumber){
void create(tecnicofs* fs, char *name, int inumber, int hashIdx){
	sync_wrlock(&(fs->bstLock[hashIdx]));
	fs->bstRoot[hashIdx] = insert(fs->bstRoot[hashIdx], name, inumber);
	sync_unlock(&(fs->bstLock[hashIdx]));
}




void delete(tecnicofs* fs, char *name, int hashIdx){
	sync_wrlock(&(fs->bstLock[hashIdx]));
	fs->bstRoot[hashIdx] = remove_item(fs->bstRoot[hashIdx], name);
	sync_unlock(&(fs->bstLock[hashIdx]));
}




int lookup(tecnicofs* fs, char *name, int hashIdx){
	sync_rdlock(&(fs->bstLock[hashIdx]));
	int inumber = 0;
	node* searchNode = search(fs->bstRoot[hashIdx], name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock[hashIdx]));
	return inumber;
}




/* Funcao de rename de um ficheiro */

void renameFile(tecnicofs* fs, char* name, char* newName, int hashIdxName, int numBuckets) {
	sync_wrlock(&(fs->bstLock[hashIdxName]));

	int hashIdxNewName = hash(newName, numBuckets);		// valor de hash do novo nome

	node* searchName = search(fs->bstRoot[hashIdxName], name);				// procura o node a ser mudado
	node* searchNewName = search(fs->bstRoot[hashIdxNewName], newName);		// procura o node que vai substituir o acima

	if(searchName != NULL && searchNewName == NULL) {						// se o primeiro for encontrado e o segundo nao, entao faz o rename
		int iNumber = fs->bstRoot[hashIdxName]->inumber;					// guarda o inumber do primeiro

		fs->bstRoot[hashIdxName] = remove_item(fs->bstRoot[hashIdxName], name);					// remove o primeiro node
		fs->bstRoot[hashIdxNewName] = insert(fs->bstRoot[hashIdxNewName], newName, iNumber);	// substitui pelo novo
	}
	sync_unlock(&(fs->bstLock[hashIdxName]));
}





void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int numBuckets){
	for(int i = 0; i < numBuckets; i++) {
		print_tree(fp, fs->bstRoot[i]);
	}
}
