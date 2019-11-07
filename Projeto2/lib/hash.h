#ifndef HASH_H
#define HASH_H 1
#include "../fs.h"
#include "bst.h"


/* Struct of a Hash Table */

typedef struct hashentry_t {
	char *key;
	tecnicofs* fs;
	struct hashentry_t *next;
} hashentry_t;

typedef struct hashtable_t {
	int size;                   	// size da tabela
	hashentry_t** hTable;  	// criacao das entradas
} hashtable_t;


int hash(char* name, int n);
hashtable_t *hashCreate(int size);
void hashInsert(hashtable_t *hashtable, char *name, int iNumber);
int hashLookup(hashtable_t *hashtable, char *name);
void hashDelete(hashtable_t *hashtable, int hashIdx, char *name);
void hashFree(hashtable_t *hashtable);

#endif

