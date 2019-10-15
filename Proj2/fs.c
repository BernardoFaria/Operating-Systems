#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>        // Threads

pthread_mutex_t mutex2;
pthread_rwlock_t rwlock;


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
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	free_tree(fs->bstRoot);
	free(fs);
}


/* Diretivas para as estrategias de sincronizacao */
void create(tecnicofs* fs, char *name, int inumber){

	#ifdef MUTEX
    	pthread_mutex_lock(&mutex2);
    #elif RWLOCK
    	pthread_rwlock_wrlock(&rwlock);
  	#endif
  
	fs->bstRoot = insert(fs->bstRoot, name, inumber);

	#ifdef MUTEX
    	pthread_mutex_unlock(&mutex2);
  	#elif RWLOCK
    	pthread_rwlock_unlock(&rwlock);
  	#endif
}


/* Diretivas para as estrategias de sincronizacao */
void delete(tecnicofs* fs, char *name){

	#ifdef MUTEX
    	pthread_mutex_lock(&mutex2);
    #elif RWLOCK
    	pthread_rwlock_wrlock(&rwlock);
  	#endif

	fs->bstRoot = remove_item(fs->bstRoot, name);

	#ifdef MUTEX
    	pthread_mutex_unlock(&mutex2);
  	#elif RWLOCK
    	pthread_rwlock_unlock(&rwlock);
  	#endif
}


/* Diretivas para as estrategias de sincronizacao */
int lookup(tecnicofs* fs, char *name){

	#ifdef MUTEX
    	pthread_mutex_lock(&mutex2);
    #elif RWLOCK
    	pthread_rwlock_rdlock(&rwlock);
  	#endif

	node* searchNode = search(fs->bstRoot, name);
	
	if ( searchNode ) {
		#ifdef MUTEX
    		pthread_mutex_unlock(&mutex2);
  		#elif RWLOCK
    		pthread_rwlock_unlock(&rwlock);
  		#endif 
		return searchNode->inumber;

	}
	#ifdef MUTEX
    	pthread_mutex_unlock(&mutex2);
  	#elif RWLOCK
    	pthread_rwlock_unlock(&rwlock);
  	#endif
	return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	print_tree(fp, fs->bstRoot);
}
