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
	fs->bstRoot = malloc(sizeof(node **)*numBuckets);
	fs->bstLock = malloc(sizeof(syncMech)*numBuckets);

	for(int i = 0; i < numBuckets; i++) {
		fs->bstRoot[i] = NULL;
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
	// hashInsert(hashtable, fs->bstRoot, name);
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





void renameFile(tecnicofs* fs, char* name, char* newName, int hashIdx) {
	sync_wrlock(&(fs->bstLock[hashIdx]));
	node* searchName = search(fs->bstRoot[hashIdx], name);
	node* searchNewName = search(fs->bstRoot[hashIdx], newName);

	if(searchName != NULL && searchNewName == NULL) {
		int iNumber = fs->bstRoot[hashIdx]->inumber;
		delete(fs, name, hashIdx);
		create(fs, newName, iNumber, hashIdx);
	}
	sync_unlock(&(fs->bstLock[hashIdx]));
}





void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int numBuckets){
	for(int i = 0; i < numBuckets; i++) {
		print_tree(fp, fs->bstRoot[i]);
	}
}
