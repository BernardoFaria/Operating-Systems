#include "hash.h"
#include "bst.h"
#include "../fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Simple hash function for strings.
 * Receives a string and resturns its hash value
 * which is a number between 0 and n-1
 * In case the string is null, returns -1 */
int hash(char* name, int n) {
	if (!name) 
		return -1;
	return (int) name[0] % n;
}


    /* Create a new hashtable. */
hashtable_t *hashCreate(int numBuckets) {

    /* verifica se o numero de entradas da tabela e valido */
	if(numBuckets < 1) {            
        return NULL;
    }

	/* Alocar memoria para a tabela */
	hashtable_t* hashtable = malloc(sizeof(hashtable_t));

	/* Alocar memoria para as entradas da tabela */
	hashtable->hTable = malloc(sizeof(hashentry_t *)*numBuckets);
	
	for(int i = 0; i < numBuckets; i++) {
		hashtable->hTable[i] = NULL;
	}

	/* Tamanho da hash table */
	hashtable->size = numBuckets;
	
	return hashtable;	
}



void hashInsert(hashtable_t *hashtable, int hashIdx, char *name, int iNumber) {
	create(hashtable->hTable[hashIdx]->fs, name, iNumber);
}



int hashLookup(hashtable_t *hashtable, int hashIdx, char *name) {
	int result = lookup(hashtable->hTable[hashIdx]->fs, name);
	return result;
}


void hashDelete(hashtable_t *hashtable, int hashIdx, char *name) {
	delete(hashtable->hTable[hashIdx]->fs, name);
}



void hashFree(hashtable_t *hashtable) {
	for(int i = 0; i < hashtable->size; i++) {
		free_tecnicofs(hashtable->hTable[i]->fs);
	}
	free(hashtable);
}