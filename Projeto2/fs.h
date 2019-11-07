/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "sync.h"
// #include "lib/hash.h" // adiciona lib

typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
    syncMech bstLock;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
// void create(tecnicofs* fs, hashTable *hashtable, int hashIndex, char *name, int inumber);       // novos argumentos
void create(tecnicofs* fs, char *name, int inumber);   
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
void renameFile(tecnicofs* fs, char* actualName, char* newName);           // nova operação
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
