/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include "fs.h"
#include "lib/bst.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sync.h"



int obtainNewInumber(tecnicofs* fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

tecnicofs* new_tecnicofs(){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->bstRoot = NULL;
	fs->nextINumber = 0;
	sync_init(&(fs->bstLock));
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	free_tree(fs->bstRoot);
	sync_destroy(&(fs->bstLock));
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber){
	sync_wrlock(&(fs->bstLock));
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	sync_unlock(&(fs->bstLock));
}

void delete(tecnicofs* fs, char *name){
	sync_wrlock(&(fs->bstLock));
	fs->bstRoot = remove_item(fs->bstRoot, name);
	sync_unlock(&(fs->bstLock));
}

int lookup(tecnicofs* fs, char *name){
	sync_rdlock(&(fs->bstLock));
	int inumber = 0;
	node* searchNode = search(fs->bstRoot, name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock));
	return inumber;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	print_tree(fp, fs->bstRoot);
}
