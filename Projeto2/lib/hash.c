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


hashentry_t *allocateEntry(char *key, tecnicofs* fs) {

	hashentry_t *entry;

	/* aloca a memoria para a nova entrada */

	if((entry = malloc(sizeof(entry))) == NULL) {
		return NULL;
	}
	if((entry->key = malloc(sizeof(key))) == NULL) {
		return NULL;
	}
	if((entry->fs = new_tecnicofs()) == NULL) {
		return NULL;
	}
	
	/* mete o proximo a null */
	entry->next = NULL;

	return entry;
} 


/* key = name */


void hashInsert(hashtable_t *hashtable, char *name, int iNumber) {

	/* Calculamos a funcao de hash */
	int hashIdx = hash(name, hashtable->size);  

	/* tentamos ver se existe um slot vazio */
	hashentry_t *entry = hashtable->hTable[hashIdx];

	/* ha slot vazio, insere imediatamente */
	if(entry == NULL) {
		hashtable->hTable[hashIdx] = allocateEntry(name, hashtable->hTable[hashIdx]->fs);
		hashtable->hTable[hashIdx]->key = name;
		create(hashtable->hTable[hashIdx]->fs, name, iNumber);
		return;
	}

	/* nao ha slot vazio */
	hashentry_t *prev;

	/* percorremos todas as entradas */
	while(entry != NULL) {
		/* vemos se a key ja existe */
		if(strcmp(entry->key, name) == 0) {
			create(hashtable->hTable[hashIdx]->fs, name, iNumber);
			return;
		}
		/* vamos para o proximo slot */
		prev = entry;
		entry = prev->next;
	}
	/* fim da hashtable, logo adicionamos */
	prev->next = allocateEntry(name, hashtable->hTable[hashIdx]->fs);
	hashtable->hTable[hashIdx]->key = name;
	create(hashtable->hTable[hashIdx]->fs, name, iNumber);

}



int hashLookup(hashtable_t *hashtable, char *name) {

	int result;

	/* Calculamos a funcao de hash */
	int hashIdx = hash(name, hashtable->size);  

	/* tentamos ver se existe este slot */
	hashentry_t *entry = hashtable->hTable[hashIdx];

	/* não existe, logo acaba */
	if(entry == NULL) return -1;

	/* se existe, vamos procura-lo */
	while(entry != NULL) {
		/* retorna-o se o encontrar */
		if(strcmp(entry->key, name) == 0) {
			result = lookup(hashtable->hTable[hashIdx]->fs, name);
			return result;
		}
		/* senao, passa ao proximo */
		entry = entry->next;
	}

	return -1;
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