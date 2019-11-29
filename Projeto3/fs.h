/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "sync.h"
#include "tecnicofs-api-constants.h"
// #include "lib/hash.h" // adiciona lib

typedef struct tecnicofs {
    node** bstRoot;         // passa a ser um vetor de nodes
    int nextINumber;
    syncMech *bstLock;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs, int numBuckets);
int create(tecnicofs* fs, char *name, int hashValue, uid_t owner, permission ownerPerm, permission othersPerm);         // novo argumento
int delete(tecnicofs* fs, char *name, int hashValue, int inumberLook, uid_t uid);
int lookup(tecnicofs* fs, char *name, int hashValue);
void renameFile(tecnicofs* fs, char* actualName, char* newName, int hashIdxName, int numBuckets, uid_t uid);           // nova operação
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs, int numBuckets);

#endif /* FS_H */
