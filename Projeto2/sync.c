/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include "sync.h"
#include <stdio.h>
#include <stdlib.h>

void sync_init(syncMech* sync){
    int ret = syncMech_init(sync, NULL);
    if(ret != 0){
        perror("sync_init failed\n");
        exit(EXIT_FAILURE);
    }
}

void sync_destroy(syncMech* sync){
    int ret = syncMech_destroy(sync);
    if(ret != 0){
        perror("sync_destroy failed\n");
        exit(EXIT_FAILURE);
    }
}

void sync_wrlock(syncMech* sync){
    int ret = syncMech_wrlock(sync);
    if(ret != 0){
        perror("sync_wrlock failed");
        exit(EXIT_FAILURE);
    }
}

void sync_rdlock(syncMech* sync){
    int ret = syncMech_rdlock(sync);
    if(ret != 0){
        perror("sync_rdlock failed");
        exit(EXIT_FAILURE);
    }
}

void sync_unlock(syncMech* sync){
    int ret = syncMech_unlock(sync);
    if(ret != 0){
        perror("sync_unlock failed");
        exit(EXIT_FAILURE);
    }
}

void mutex_init(pthread_mutex_t* mutex){
    #if defined (RWLOCK) || defined (MUTEX)
        int ret = pthread_mutex_init(mutex, NULL);
        if(ret != 0){
            perror("mutex_init failed\n");
            exit(EXIT_FAILURE);
        }
    #endif
}

void mutex_destroy(pthread_mutex_t* mutex){
    #if defined (RWLOCK) || defined (MUTEX)
        int ret = pthread_mutex_destroy(mutex);
        if(ret != 0){
            perror("mutex_destroy failed\n");
            exit(EXIT_FAILURE);
        }
    #endif
}

void mutex_lock(pthread_mutex_t* mutex){
    #if defined (RWLOCK) || defined (MUTEX)
        int ret = pthread_mutex_lock(mutex);
        if(ret != 0){
            perror("mutex_lock failed");
            exit(EXIT_FAILURE);
        }
    #endif
}

void mutex_unlock(pthread_mutex_t* mutex){
    #if defined (RWLOCK) || defined (MUTEX)
        int ret = pthread_mutex_unlock(mutex);
        if(ret != 0){
            perror("mutex_unlock failed");
            exit(EXIT_FAILURE);
        }
     #endif
}

int do_nothing(void* a){
    (void)a;
    return 0;
}
